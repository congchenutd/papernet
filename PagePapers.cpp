#include "PagePapers.h"
#include "Common.h"
#include "PaperDlg.h"
#include "QuoteDlg.h"
#include "Navigator.h"
#include "TagDlg.h"
#include "MainWindow.h"
#include "RefFormatSpec.h"
#include "RefExporter.h"
#include "RefParser.h"
#include "PaperList.h"
#include "OptionDlg.h"
#include "WebImporter.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileInfo>
#include <QProgressDialog>
#include <QClipboard>
#include <QProcess>
#include <QDate>

PagePapers::PagePapers(QWidget *parent)
	: Page(parent)
{
    _currentRow     = -1;
	_currentPaperID = -1;
	_setting = UserSetting::getInstance();

	ui.setupUi(this);

    ui.tvPapers->init("PagePapers", _setting);   // set the table name for the view
    resetModel();   // init model, table ...

	QDataWidgetMapper* mapper = new QDataWidgetMapper(this);
	mapper->setModel(&_model);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper->addMapping(ui.teNote,     PAPER_NOTE);

	ui.tvPapers->setModel(&_model);
    ui.tvPapers->hideColumn(PAPER_ID);
    for(int col = PAPER_TYPE; col <= PAPER_NOTE; ++col)
        ui.tvPapers->hideColumn(col);

	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);
    ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);    // it's just icon
	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);

    // tag table, relation table, foreign key in the relation table
	ui.widgetWordCloud->setTableNames("Tags", "PaperTag", "Paper");

	loadGeometry();

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			mapper, SLOT(setCurrentModelIndex(QModelIndex)));
    connect(ui.tvPapers->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection)));

    connect(ui.tvPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
            this, SLOT(onSubmitPaper()));  // submit before sorting
	connect(ui.tvPapers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditPaper()));
    connect(ui.tvPapers, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked()));
	connect(ui.tvPapers, SIGNAL(addQuote()),     this, SLOT(onAddQuote()));
	connect(ui.tvPapers, SIGNAL(printMe(bool)),  this, SLOT(onPrintMe(bool)));
	connect(ui.tvPapers, SIGNAL(bookmark(bool)), this, SLOT(onBookmark(bool)));
	connect(ui.tvPapers, SIGNAL(addPDF()),       this, SLOT(onAddPDF()));
	connect(ui.tvPapers, SIGNAL(readPDF()),      this, SLOT(onReadPDF()));

	connect(ui.widgetWordCloud, SIGNAL(filter(bool)), this, SLOT(onFilterPapersByTags(bool)));
    connect(ui.widgetWordCloud, SIGNAL(unfilter()),   this, SLOT(resetAndJumpToCurrent()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onNewTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagToPaper()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPaper()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));

    connect(ui.widgetRelated,    SIGNAL(doubleClicked(int)), this, SLOT(onRelatedDoubleClicked(int)));
    connect(ui.widgetCoauthered, SIGNAL(doubleClicked(int)), this, SLOT(onRelatedDoubleClicked(int)));
    connect(ui.widgetQuotes,     SIGNAL(quotesChanged()),    this, SLOT(updateQuotes()));
}

void PagePapers::onSelectionChanged(const QItemSelection& selected)
{
    if(!selected.isEmpty())
    {
        _currentRow = selected.indexes().front().row();
        _currentPaperID = rowToID(_currentRow);
        highLightTags();                                        // update tags
        reloadAttachments();                                    // attachments
        ui.widgetRelated   ->setCentralPaper(_currentPaperID);  // related
        ui.widgetCoauthered->setCentralPaper(_currentPaperID);  // coauthored
        updateQuotes();                                         // quotes
        emit hasPDF(isAttached(_currentPaperID) >= ATTACH_PAPER);
    }
    emit selectionValid(!selected.isEmpty());
}

// only triggered by mouse click, not programmatically
void PagePapers::onClicked() {
	Navigator::getInstance()->addFootStep(this, _currentPaperID);
}

void PagePapers::addRecord()
{
    PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Add Reference"));
	if(dlg.exec() == QDialog::Accepted)
        insertReference(dlg.getReference());
}

void PagePapers::onEditPaper()
{
    PaperDlg dlg(this);
    dlg.setWindowTitle(tr("Edit Reference"));
    Reference oldRef = exportReference(_currentRow);
    dlg.setReference(oldRef);

    connect(&dlg, SIGNAL(selectPaper(int)), this, SLOT(onJumpToCurrent(int)));

    if(dlg.exec() == QDialog::Accepted)
    {
        Reference newRef = dlg.getReference();
        updateReference(_currentRow, newRef);   // apply the change

        // renaming title affects attachments
        QString oldTitle = oldRef.getValue("title").toString();
        QString newTitle = newRef.getValue("title").toString();
        if(oldTitle != newTitle)
        {
            ::renameTitle(oldTitle, newTitle);
            reloadAttachments();           // refresh attached files after renaming
        }

        if(newRef.getValue("note") != oldRef.getValue("note"))
            setPaperRead();                // changing note infers being read
    }
}

void PagePapers::insertReference(const Reference& ref)
{
    resetModel();  // ensure the newly inserted is visible
                   // don't call it in updateReference(),
                   // because the ID (by model.insertRow()) is not submitted yet

    // search title, if exists, replace, otherwise, insert
    int row = titleToRow(ref.getValue("title").toString());
    if(row > -1) {  // merge to existing paper
        updateReference(row, ref);
    }
    else            // insert as a new one
    {
        int lastRow = _model.rowCount();
        _model.insertRow(lastRow);
        _currentPaperID = getNextID("Papers", "ID");
        _model.setData(_model.index(lastRow, PAPER_ID), _currentPaperID);
        updateReference(lastRow, ref);
        onBookmark(true);    // attach the ReadMe tag
    }
}

void PagePapers::updateReference(int row, const Reference& ref)
{
    if(row < 0 || row > _model.rowCount())
        return;

    QMap<int, QString> fields;    // column -> name
    fields.insert(PAPER_YEAR,        "year");
    fields.insert(PAPER_VOLUME,      "volume");
    fields.insert(PAPER_ISSUE,       "issue");
    fields.insert(PAPER_STARTPAGE,   "startpage");
    fields.insert(PAPER_ENDPAGE,     "endpage");
    fields.insert(PAPER_TITLE,       "title");
    fields.insert(PAPER_TYPE,        "type");
    fields.insert(PAPER_PUBLICATION, "publication");
    fields.insert(PAPER_PUBLISHER,   "publisher");
    fields.insert(PAPER_EDITORS,     "editors");
    fields.insert(PAPER_ADDRESS,     "address");
    fields.insert(PAPER_URL,         "url");
    fields.insert(PAPER_ABSTRACT,    "abstract");
    fields.insert(PAPER_NOTE,        "note");

    for(QMap<int, QString>::iterator it = fields.begin(); it != fields.end(); ++it)
        _model.setData(_model.index(row, it.key()), ref.getValue(it.value()));

    // authors is a QStringList
    _model.setData(_model.index(row, PAPER_AUTHORS),
                   ref.getValue("authors").toStringList().join("; "));

    // tags are stored separately in a relations table
    recreateTagsRelations(ref.getValue("tags").toStringList());

    // modified date
    _model.setData(_model.index(row, PAPER_MODIFIED), QDate::currentDate().toString("yyyy/MM/dd"));

	onSubmitPaper();
}

void PagePapers::recreateTagsRelations(const QStringList& tags)
{
    if(tags.isEmpty())
        return;

	// remove all relations to tags
	QSqlQuery query;
	query.exec(tr("delete from PaperTag where Paper = %1").arg(_currentPaperID));

	// add relations back
	foreach(QString tagName, tags)
		attachNewTag(tagName);

	highLightTags();
}

void PagePapers::delRecord()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tvPapers->selectionModel()->selectedRows();
        QSqlDatabase::database().transaction();
        foreach(QModelIndex idx, idxList)     // find all selected indexes
            delPaper(rowToID(idx.row()));     // delete in the db
        QSqlDatabase::database().commit();
        _model.select();                      // reload db
	}
}

int PagePapers::rowToID(int row) const {
    return row > -1 && row < _model.rowCount() ?
                _model.data(_model.index(row, PAPER_ID)).toInt() : -1;
}

int PagePapers::titleToRow(const QString& title) const
{
    QModelIndexList indexes = _model.match(_model.index(0, PAPER_TITLE),
                                          Qt::DisplayRole, title, 1, Qt::MatchFixedString);
    return indexes.isEmpty() ? -1 : indexes.front().row();
}

void PagePapers::importFromFiles(const QStringList& filePaths)
{
    foreach(const QString& filePath, filePaths)
    {
		// reference only
        QString extension = QFileInfo(filePath).suffix().toLower();
        if((extension != "bib" && extension != "enw" && extension != "ris") || extension == "pdf")
            continue;

        // find spec
        RefSpec* spec = RefSpecFactory::getInstance()->getSpec(extension);
        if(!spec)
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Can not find the specification for this reference format!"));
            return;
        }

        // open file
        QFile file(filePath);
        if(!file.open(QFile::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Can not open file %1").arg(filePath));
            return;
        }

        // parse
        QList<Reference> references = spec->getParser()->parse(file.readAll(), spec);
        importReferences(references);  // currentPaperID = -1 if failed
    }

	// pdfs
    foreach(QString filePath, filePaths)
        if(QFileInfo(filePath).suffix().toLower() == "pdf")
            importPDF(filePath);
}

void PagePapers::importReferences(const QList<Reference>& references)
{
    if(references.isEmpty())
        return;

    QSqlDatabase::database().transaction();
    foreach(const Reference& ref, references)
    {
        QString title = ref.getValue("title").toString();
        if(title.isEmpty())
            continue;

        // show preview
        PaperDlg dlg(this);
        dlg.setWindowTitle(tr("Import Reference"));

        // load existing ref
        int row = titleToRow(title);
        if(row > -1)
        {
            Reference oldRef = exportReference(row);
            dlg.setReference(oldRef);
        }

        dlg.setReference(ref);   // merge new ref
        dlg.showMergeMark();

        if(dlg.exec() == QDialog::Accepted)
            insertReference(dlg.getReference());   // currentPaperID will be equal to the ID of the newly added
    }
    QSqlDatabase::database().commit();
}

void PagePapers::importPDF(const QString& pdfPath)
{
    PaperList dlg(this);
	dlg.setWindowTitle(tr("To which paper will %1 be attached?").arg(QFileInfo(pdfPath).fileName()));
    if(dlg.exec() == QDialog::Accepted && !dlg.getSelected().isEmpty())
    {
        ui.tabWidget->setCurrentWidget(ui.tabAttachments);  // show attachment tab
        resetModel();                                       // ensure the paper is visible
        jumpToID(getPaperID(dlg.getSelected().front()));    // jump to the paper, and update _currentPaperID
        addAttachment(_currentPaperID, suggestAttachmentName(pdfPath), pdfPath);
        reloadAttachments();
    }
}

QList<Reference> PagePapers::parseContent(const QString &content)
{
    // try all specs
    QFileInfoList fileInfos = QDir("./Specifications").entryInfoList(QStringList() << "*.spec");
    foreach(QFileInfo fileInfo, fileInfos)
        if(RefSpec* spec = RefSpecFactory::getInstance()->getSpec(fileInfo.baseName()))
        {
            // use the parser associated with the spec
            QList<Reference> references = spec->getParser()->parse(content, spec);
            if(!references.isEmpty())
                return references;
        }
    return QList<Reference>();   // no parser can parse
}

void PagePapers::onImport()
{
    // try clipboard first
    QClipboard* clipboard = QApplication::clipboard();
    QString content = clipboard->text();
    if(!content.isEmpty())
    {
        QUrl url(content);
        if(url.isValid())
        {
            WebImporter* webImporter = WebImporter::getInstance();
            if(webImporter->parse(url))
            {
                Reference ref = webImporter->getReference();
                QString   pdfPath = webImporter->getTempPDFPath();
//                importReferences(QList<Reference>() << ref);
//                addAttachment(_currentPaperID, suggestAttachmentName(pdfPath), pdfPath);
            }
        }

        QList<Reference> references = parseContent(content);
        if(!references.isEmpty())
        {
            importReferences(references);
            return;
        }
    }

    // get input files
    QStringList files = QFileDialog::getOpenFileNames(
                            this, "Import references", _setting->getLastImportPath(),
							"Reference (*.enw *.ris *.bib *.pdf);;All files (*.*)");
    if(files.isEmpty())
        return;
    _setting->setLastImportPath(QFileInfo(files.front()).absolutePath());

    importFromFiles(files);
}

void PagePapers::search(const QString& target)
{
	// filter papers
	_model.setFilter(
                tr("Title       like \"%%1%\" or \
                    Authors     like \"%%1%\" or \
                    Year        like \"%%1%\" or \
                    Publication like \"%%1%\" or \
                    Abstract    like \"%%1%\" or \
                    Note        like \"%%1%\" ").arg(target));

	// highlight tags
	ui.widgetWordCloud->search(target);
}

void PagePapers::onNewTag()
{
	TagDlg dlg("Tags", this);
	if(dlg.exec() == QDialog::Accepted)
	{
		int tagID = getNextID("Tags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());         // create tag
		ui.widgetWordCloud->addTagToItem(tagID, _currentPaperID);  // add to paper
		highLightTags();
	}
}

void PagePapers::onAddTagToPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)  // for all selected papers
	{
		int paperID = _model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)   // add all selected tags to selected papers
			ui.widgetWordCloud->addTagToItem(getTagID("Tags", tag->text()), paperID);
	}
	highLightTags();
}

// highlight the tags of current paper
void PagePapers::highLightTags() {
	ui.widgetWordCloud->highLight(getTagsOfPaper(_currentPaperID));
}

// same structure as onAddTagToPaper()
void PagePapers::onDelTagFromPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
		int paperID = _model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", tag->text()), paperID);
	}
	highLightTags();
}

// submit the model
// keep selecting current paper
void PagePapers::onSubmitPaper()
{
    if(!_model.submitAll())
        QMessageBox::critical(this, tr("Error"), _model.lastError().text());
    jumpToCurrent();
}

void PagePapers::resetAndJumpToCurrent()
{
    resetModel();
    jumpToCurrent();
}

void PagePapers::resetModel()
{
    _model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    _model.setTable("Papers");
    _model.select();
    _model.setHeaderData(PAPER_ATTACHED, Qt::Horizontal, "@");
    ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
}

void PagePapers::jumpToID(int id)
{
    _model.select();
    while(_model.canFetchMore())    // ensure all records visible
        _model.fetchMore();

    _currentRow = idToRow(&_model, PAPER_ID, id);
    if(_currentRow < 0)
        _currentRow = 0;
    ui.tvPapers->selectRow(_currentRow);  // will trigger onSelectionChanged()
    ui.tvPapers->scrollTo(_model.index(_currentRow, PAPER_TITLE));
    ui.tvPapers->setFocus();
}

// filter papers with tags
void PagePapers::onFilterPapersByTags(bool AND)
{
	// get selected tags
	QStringList tagIDs;
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)
		tagIDs << tr("%1").arg(getTagID("Tags", tag->text()));

	if(!AND)
		_model.setFilter(
			tr("ID in (select Paper from PaperTag where Tag in (%1))").arg(tagIDs.join(",")));
	else
	{
		dropTempView();
		QSqlQuery query;    // create a temp table for selected tags id
		query.exec(tr("create view SelectedTags as \
					   select * from Tags where ID in (%1)").arg(tagIDs.join(",")));

		// select papers that contain all the selected tags
		_model.setFilter("not exists \
						(select * from SelectedTags where \
							 not exists \
							 (select * from PaperTag where \
							 Paper=Papers.ID and Tag=SelectedTags.ID))");
	}
}

void PagePapers::updateQuotes() {
    ui.widgetQuotes->setCentralPaper(_currentPaperID);
}

void PagePapers::onAddQuote()
{
    QuoteDlg dlg(this);
    dlg.setWindowTitle(tr("Add Quote"));
    dlg.setQuoteID(getNextID("Quotes", "ID"));    // create new quote id
    dlg.addRef(getPaperTitle(_currentPaperID));   // add current paper
    if(dlg.exec() == QDialog::Accepted)
        updateQuotes();
}

void PagePapers::onAddPDF()
{
	ui.tabWidget->setCurrentWidget(ui.tabAttachments);  // show attachment tab
	ui.widgetAttachments->onAddFile();
}

void PagePapers::onReadPDF() {
    openAttachment(_currentPaperID, "Paper.pdf");
}

void PagePapers::onJumpToCurrent(int id) {
    jumpToID(id);
}

void PagePapers::onFullTextSearch(const QString& target)
{
    resetModel();
	if(target.isEmpty())
		return;

	int rowCount = _model.rowCount();
	QProgressDialog progress(tr("Searching..."), tr("Abort"), 0, rowCount, this);
	progress.setWindowModality(Qt::WindowModal);

	QStringList filter;
	for(int row = 0; row < rowCount; ++row)
	{
		progress.setValue(row);
		qApp->processEvents();
		if(fullTextSearch(rowToID(row), target))        // if found in this paper
			filter << tr("ID = %1").arg(rowToID(row));  // add the paper to filter
		if(progress.wasCanceled())
			break;
	}

	if(filter.isEmpty())
		QMessageBox::information(this, tr("Full text search"), tr("No such paper!"));
	else
		_model.setFilter(filter.join(" OR "));        // filter out papers
}

void PagePapers::loadGeometry()
{
	ui.splitterHorizontal->restoreState(_setting->getSplitterSizes("PapersHorizontal"));
	ui.splitterPapers    ->restoreState(_setting->getSplitterSizes("PapersVertical"));
	ui.tabWidget->setCurrentIndex(_setting->getPapersTabIndex());
}

void PagePapers::saveGeometry()
{
    // closeEvent() does not work for child windows
    ui.tvPapers->saveSectionSizes();
    ui.widgetRelated->saveSectionSizes();
    ui.widgetCoauthered->saveSectionSizes();

	// splitters
	_setting->setSplitterSizes("PapersHorizontal", ui.splitterHorizontal->saveState());
	_setting->setSplitterSizes("PapersVertical",   ui.splitterPapers    ->saveState());

	_setting->setPapersTabIndex(ui.tabWidget->currentIndex());
}

void PagePapers::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
        resetAndJumpToCurrent();
	else
		onFilterPapersByTags();
}

void PagePapers::reloadAttachments() {
	ui.widgetAttachments->setPaper(_currentPaperID);
}

void PagePapers::onPrintMe(bool print)
{
	if(print)
		attachNewTag("PrintMe");
	else
		ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", "PrintMe"), _currentPaperID);
	highLightTags();
}

void PagePapers::onBookmark(bool readMe)
{
	if(readMe)
		attachNewTag("ReadMe");
	else
		setPaperRead();
	highLightTags();
}

void PagePapers::attachNewTag(const QString& tagName)
{
	int tagID = getTagID("Tags", tagName);
	if(tagID < 0)   // new tag
	{
		tagID = getNextID("Tags", "ID");
		ui.widgetWordCloud->addTag(tagID, tagName);
	}
    ui.widgetWordCloud->addTagToItem(tagID, _currentPaperID);
}

void PagePapers::setPaperRead()
{
	ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", "ReadMe"), _currentPaperID);
	highLightTags();
}

void PagePapers::onRelatedDoubleClicked(int paperID)
{
    _currentRow = idToRow(&_model, PAPER_ID, paperID);
    onEditPaper();
}

void PagePapers::onQuoteDoubleClicked(int quoteID) {
	MainWindow::getInstance()->jumpToQuote(quoteID);
}

// row -> Reference
Reference PagePapers::exportReference(int row) const
{
    Reference ref;
	QSqlRecord record = _model.record(row);
    for(int col = 0; col < record.count(); ++col)
    {
        QString fieldName = record.fieldName(col).toLower();
        if(fieldName == "attached")  // not exported
			continue;
        QVariant fieldValue = record.value(col);
        if(fieldName == "authors")     // to QStringList
            ref.setValue(fieldName, splitAuthorsList(fieldValue.toString()));
		else
            ref.setValue(fieldName, fieldValue);
    }

    ref.setValue("tags", getTagsOfPaper(rowToID(row)));  // tags are not in the paper table
    return ref;
}

QString PagePapers::toString(const QModelIndexList& idxList, const QString& extension)
{
    // find spec
    RefSpec* spec = RefSpecFactory::getInstance()->getSpec(extension);
    if(spec == 0)
        return tr("Cannot find the specification for this reference format!");

    // get the output
    IRefExporter* exporter = ExporterFactory::getInstance()->getExporter(extension);
    QString result;
    QTextStream os(&result);
    foreach(QModelIndex idx, idxList)    // save each row
    {
        Reference ref = exportReference(idx.row());
        ref.generateID();
        os << exporter->toString(ref, *spec) << "\r\n";
    }
    return result;
}

void PagePapers::onExport()
{
    // get selected rows
    QModelIndexList idxSelected = ui.tvPapers->selectionModel()->selectedRows();
    if(idxSelected.isEmpty())
        return;    

    // save to BibFixer
    if(_setting->getExportToBibFixer())
    {
        QString bibFixerPath = _setting->getBibFixerPath();
        if(bibFixerPath.isEmpty() || !QFile::exists(bibFixerPath))
        {
            QMessageBox::critical(this, tr("Error"), tr("Cannot find BibFixer!"));
            return;
        }

        QProcess* process = new QProcess(this);
        process->setWorkingDirectory(QFileInfo(bibFixerPath).path());
        process->start(bibFixerPath, QStringList() << toString(idxSelected, "bib"));
    }
    // save to file
    else
    {
        // get file name
        QString lastPath = _setting->getLastImportPath();
        QString filePath = QFileDialog::getSaveFileName(this, tr("Export reference"),
                                                        lastPath + "/" + getPaperTitle(_currentPaperID),
                                                        "Bibtex (*.bib);;Endnote (*.enw);;Reference Manager (*.ris);;All files (*.*)");
        if(filePath.isEmpty())
            return;
        _setting->setLastImportPath(QFileInfo(filePath).absolutePath());

        QFile file(filePath);
        if(file.open(QFile::WriteOnly | QFile::Truncate))
        {
            QTextStream os(&file);
            os << toString(idxSelected, QFileInfo(filePath).suffix().toLower());
        }
    }
}
