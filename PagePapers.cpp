#include "PagePapers.h"
#include "Common.h"
#include "PaperDlg.h"
#include "AddQuoteDlg.h"
#include "Navigator.h"
#include "AddTagDlg.h"
#include "MainWindow.h"
#include "RefFormatSpec.h"
#include "RefExporter.h"
#include "PaperList.h"
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

PagePapers::PagePapers(QWidget *parent)
	: Page(parent)
{
	currentRow = -1;
	currentPaperID = -1;
	setting = UserSetting::getInstance();

	ui.setupUi(this);
	ui.tvPapers->init("PagePapers");   // set the table name for the view

	onResetPapers();   // init model, table ...

	QDataWidgetMapper* mapper = new QDataWidgetMapper(this);
	mapper->setModel(&model);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper->addMapping(ui.teNote,     PAPER_NOTE);

	ui.tvPapers->setModel(&model);
	ui.tvPapers->hideColumn(PAPER_ID);
	for(int col = PAPER_TYPE; col <= PAPER_NOTE; ++col)
		ui.tvPapers->hideColumn(col);
	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);
	ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);
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
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),   this, SLOT(onResetPapers()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onNewTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagToPaper()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPaper()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));

    connect(ui.widgetRelated,    SIGNAL(doubleClicked(int)), this, SLOT(onRelatedDoubleClicked(int)));
    connect(ui.widgetCoauthered, SIGNAL(doubleClicked(int)), this, SLOT(onRelatedDoubleClicked(int)));
}

void PagePapers::onSelectionChanged(const QItemSelection& selected)
{
    if(!selected.isEmpty())
    {
        currentRow = selected.indexes().front().row();
        currentPaperID = rowToID(currentRow);
        highLightTags();
        reloadAttachments();
        ui.widgetRelated   ->setCentralPaper(currentPaperID);
        ui.widgetCoauthered->setCentralPaper(currentPaperID);
        ui.widgetQuotes    ->setCentralPaper(currentPaperID);
        emit hasPDF(isAttached(currentPaperID) >= ATTACH_PAPER);
    }
    emit selectionValid(!selected.isEmpty());
}

// only triggered by mouse click, not programmatically
void PagePapers::onClicked() {
	Navigator::getInstance()->addFootStep(this, currentPaperID);
}

void PagePapers::jumpToID(int id)
{
    while(model.canFetchMore())
		model.fetchMore();
	currentRow = idToRow(&model, PAPER_ID, id);
	if(currentRow < 0)
		currentRow = 0;
    ui.tvPapers->selectRow(currentRow);  // will trigger onSelectionChanged()
	ui.tvPapers->scrollTo(model.index(currentRow, PAPER_TITLE));
    ui.tvPapers->setFocus();
}

void PagePapers::add()
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
	Reference oldRef = exportReference(currentRow);
    dlg.setReference(oldRef);

	if(dlg.exec() == QDialog::Accepted)
	{
		Reference newRef = dlg.getReference();
		updateReference(currentRow, newRef);   // apply the change

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
    reset();   // do not call reset() in updateReference(),
               // because model.insertRow() is not submitted yet

    // search title, if exists, replace, otherwise, insert
    int row = titleToRow(ref.getValue("title").toString());
    if(row > -1) {  // merge to existing paper
        updateReference(row, ref);
    }
    else            // insert as a new one
    {
        int lastRow = model.rowCount();
        model.insertRow(lastRow);
        model.setData(model.index(lastRow, PAPER_ID), getNextID("Papers", "ID"));
        updateReference(lastRow, ref);
        onBookmark(true);    // attach the ReadMe tag
    }
}

void PagePapers::updateReference(int row, const Reference& ref)
{
    currentPaperID = rowToID(row);

    QMap<int, QString> intFields;
    intFields.insert(PAPER_YEAR,      "year");
    intFields.insert(PAPER_VOLUME,    "volume");
    intFields.insert(PAPER_ISSUE,     "issue");
    intFields.insert(PAPER_STARTPAGE, "startpage");
    intFields.insert(PAPER_ENDPAGE,   "endpage");

    QMap<int, QString> stringFields;
    stringFields.insert(PAPER_TITLE,       "title");
    stringFields.insert(PAPER_TYPE,        "type");
    stringFields.insert(PAPER_PUBLICATION, "publication");
    stringFields.insert(PAPER_ABSTRACT,    "abstract");
    stringFields.insert(PAPER_PUBLISHER,   "publisher");
    stringFields.insert(PAPER_EDITORS,     "editors");
    stringFields.insert(PAPER_ADDRESS,     "address");
    stringFields.insert(PAPER_URL,         "url");
    stringFields.insert(PAPER_NOTE,        "note");

    // replace integer fields
    for(QMap<int, QString>::iterator it = intFields.begin(); it != intFields.end(); ++it)
        model.setData(model.index(row, it.key()), ref.getValue(it.value()));

    // replace string fields when the old data is empty and the new one valid
    for(QMap<int, QString>::iterator it = stringFields.begin(); it != stringFields.end(); ++it)
    {
        if(model.data(model.index(row, it.key())).toString().isEmpty() &&
           ref.getValue(it.value()).isValid())
            model.setData(model.index(row, it.key()), ref.getValue(it.value()));
    }

    // authors is a QStringList
    model.setData(model.index(row, PAPER_AUTHORS),
                  ref.getValue("authors").toStringList().join("; "));

    // tags are stored in a relations table
    updateTags(ref.getValue("tags").toStringList());

	onSubmitPaper();
}

void PagePapers::updateTags(const QStringList& tags)
{
    if(tags.isEmpty())
        return;

	// remove all relations to tags
	QSqlQuery query;
	query.exec(tr("delete from PaperTag where Paper = %1").arg(currentPaperID));

	// add relations back
	foreach(QString tagName, tags)
		attachNewTag(tagName);

	highLightTags();
}

void PagePapers::del()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tvPapers->selectionModel()->selectedRows();
        QSqlDatabase::database().transaction();
        foreach(QModelIndex idx, idxList)     // find all selected indexes
            delPaper(rowToID(idx.row()));     // delete in the db
        QSqlDatabase::database().commit();
        model.select();                       // reload db
	}
}

int PagePapers::rowToID(int row) const {
    return row > -1 && row < model.rowCount() ?
                model.data(model.index(row, PAPER_ID)).toInt() : -1;
}

int PagePapers::titleToRow(const QString& title) const
{
    QModelIndexList indexes = model.match(model.index(0, PAPER_TITLE),
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
        RefFormatSpec* spec = SpecFactory::getInstance()->getSpec(extension);
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
        QList<Reference> references = spec->parse(file.readAll());
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
        dlg.setWindowTitle(tr("Import reference"));
        dlg.setReference(ref);
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
        jumpToID(getPaperID(dlg.getSelected().front()));
        addAttachment(currentPaperID, suggestAttachmentName(pdfPath), pdfPath);
        reloadAttachments();
    }
}

void PagePapers::onImport()
{
    // try clipboard first
    QClipboard* clipboard = QApplication::clipboard();
    QString content = clipboard->text();
    if(!content.isEmpty())
    {
        QList<Reference> references = SpecFactory::getInstance()->parseContent(content);
        if(!references.isEmpty())
        {
            importReferences(references);
            return;
        }
    }

    // get input files
    QStringList files = QFileDialog::getOpenFileNames(
                            this, "Import references", setting->getLastImportPath(),
							"Reference (*.enw *.ris *.bib *.pdf);;All files (*.*)");
    if(files.isEmpty())
        return;
    setting->setLastImportPath(QFileInfo(files.front()).absolutePath());

    importFromFiles(files);
}

// submit the model
// keep selecting current paper
void PagePapers::onSubmitPaper()
{
	int backup = currentPaperID;
	if(!model.submitAll())
		QMessageBox::critical(this, tr("Error"), model.lastError().text());
	currentPaperID = backup;
	jumpToID(backup);
}

void PagePapers::search(const QString& target)
{
	// filter papers
	model.setFilter(
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
	AddTagDlg dlg("Tags", this);
	if(dlg.exec() == QDialog::Accepted)
	{
		int tagID = getNextID("Tags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());         // create tag
		ui.widgetWordCloud->addTagToItem(tagID, currentPaperID);  // add to paper
		highLightTags();
	}
}

void PagePapers::onAddTagToPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)  // for all selected papers
	{
		int paperID = model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)   // add all selected tags to selected papers
			ui.widgetWordCloud->addTagToItem(getTagID("Tags", tag->text()), paperID);
	}
	highLightTags();
}

// highlight the tags of current paper
void PagePapers::highLightTags() {
	ui.widgetWordCloud->highLight(getTagsOfPaper(currentPaperID));
}

// same structure as onAddTagToPaper()
void PagePapers::onDelTagFromPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
		int paperID = model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", tag->text()), paperID);
	}
	highLightTags();
}

void PagePapers:: onResetPapers()
{
//    dropTempView();   // why needed?

	int backupID = currentPaperID;
	model.setEditStrategy(QSqlTableModel::OnManualSubmit);
	model.setTable("Papers");
	model.select();
	model.setHeaderData(PAPER_ATTACHED, Qt::Horizontal, "@");

	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);

	currentPaperID = backupID;
	jumpToCurrent();
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
		model.setFilter(
			tr("ID in (select Paper from PaperTag where Tag in (%1))").arg(tagIDs.join(",")));
	else
	{
		dropTempView();
		QSqlQuery query;    // create a temp table for selected tags id
		query.exec(tr("create view SelectedTags as \
					   select * from Tags where ID in (%1)").arg(tagIDs.join(",")));

		// select papers that contain all the selected tags
		model.setFilter("not exists \
						(select * from SelectedTags where \
							 not exists \
							 (select * from PaperTag where \
							 Paper=Papers.ID and Tag=SelectedTags.ID))");
	}
}

void PagePapers::onAddQuote()
{
    AddQuoteDlg dlg(this);
    connect(&dlg, SIGNAL(accepted()), this, SLOT(onResetPapers()));
    dlg.setWindowTitle(tr("Add Quote"));
    dlg.setQuoteID(getNextID("Quotes", "ID"));    // create new quote id
    dlg.addRef(getPaperTitle(currentPaperID));    // add itself
	dlg.exec();
}

void PagePapers::onAddPDF()
{
	ui.tabWidget->setCurrentWidget(ui.tabAttachments);  // show attachment tab
	ui.widgetAttachments->onAddFile();
}

void PagePapers::onReadPDF() {
	openAttachment(currentPaperID, "Paper.pdf");
}

void PagePapers::onFullTextSearch(const QString& target)
{
	onResetPapers();
	if(target.isEmpty())
		return;

	int rowCount = model.rowCount();
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
		model.setFilter(filter.join(" OR "));        // filter out papers
}

void PagePapers::loadGeometry()
{
	ui.splitterHorizontal->restoreState(setting->getSplitterSizes("PapersHorizontal"));
	ui.splitterPapers    ->restoreState(setting->getSplitterSizes("PapersVertical"));
	ui.tabWidget->setCurrentIndex(setting->getPapersTabIndex());
}

void PagePapers::saveGeometry()
{
	ui.tvPapers->saveSectionSizes();  // sections

	// splitters
	setting->setSplitterSizes("PapersHorizontal", ui.splitterHorizontal->saveState());
	setting->setSplitterSizes("PapersVertical",   ui.splitterPapers    ->saveState());
	setting->setPapersTabIndex(ui.tabWidget->currentIndex());
}

void PagePapers::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		onResetPapers();
	else
		onFilterPapersByTags();
}

void PagePapers::reloadAttachments() {
	ui.widgetAttachments->setPaper(currentPaperID);
}

void PagePapers::onPrintMe(bool print)
{
	if(print)
		attachNewTag("PrintMe");
	else
		ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", "PrintMe"), currentPaperID);
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
    ui.widgetWordCloud->addTagToItem(tagID, currentPaperID);
}

void PagePapers::setPaperRead()
{
	ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", "ReadMe"), currentPaperID);
	highLightTags();
}

void PagePapers::onRelatedDoubleClicked(int paperID)
{
	reset();
	jumpToID(paperID);
	Navigator::getInstance()->addFootStep(this, paperID);
}

void PagePapers::onQuoteDoubleClicked(int quoteID) {
	MainWindow::getInstance()->jumpToQuote(quoteID);
}

// row -> Reference
Reference PagePapers::exportReference(int row) const
{
    Reference ref;
	QSqlRecord record = model.record(row);
    for(int col = 0; col < record.count(); ++col)
    {
        QString fieldName = record.fieldName(col).toLower();
        if(fieldName == "id" || fieldName == "attached")  // not exported
			continue;
        QVariant fieldValue = record.value(col);
        if(fieldName == "authors")
            ref.setValue(fieldName, splitAuthorsList(fieldValue.toString()));  // to QStringList
		else
            ref.setValue(fieldName, fieldValue);
    }

    ref.setValue("tags", getTagsOfPaper(rowToID(row)));  // NOTE: tags is not stored in the paper table
    return ref;
}

QString PagePapers::toString(const QModelIndexList& idxList, const QString& extension)
{
    // find spec
    RefFormatSpec* spec = SpecFactory::getInstance()->getSpec(extension);
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
    QModelIndexList idxList = ui.tvPapers->selectionModel()->selectedRows();
    if(idxList.isEmpty())
        return;    

    // save to BibFixer
    if(setting->getExportToBibFixer())
    {
        QString bibFixerPath = setting->getBibFixerPath();
        if(bibFixerPath.isEmpty())
        {
            QMessageBox::critical(this, tr("Error"), tr("Cannot find BibFixer!"));
            return;
        }

        QProcess* process = new QProcess(this);
        process->setWorkingDirectory(QFileInfo(bibFixerPath).path());
        process->start(bibFixerPath, QStringList() << toString(idxList, "bib"));
    }
    // save to file
    else
    {
        // get file name
        QString lastPath = setting->getLastImportPath();
        QString filePath = QFileDialog::getSaveFileName(this, tr("Export reference"),
                                                        lastPath + "/" + getPaperTitle(currentPaperID),
                                                        "Bibtex (*.bib);;Endnote (*.enw);;Reference Manager (*.ris);;All files (*.*)");
        if(filePath.isEmpty())
            return;
        setting->setLastImportPath(QFileInfo(filePath).absolutePath());

        QFile file(filePath);
        if(file.open(QFile::WriteOnly | QFile::Truncate))
        {
            QTextStream os(&file);
            os << toString(idxList, QFileInfo(filePath).suffix().toLower());
        }
    }
}
