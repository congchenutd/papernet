#include "PagePapers.h"
#include "Common.h"
#include "PaperDlg.h"
#include "AddQuoteDlg.h"
#include "Importer.h"
#include "Navigator.h"
#include "AddTagDlg.h"
#include "MainWindow.h"
#include "RefFormatSpec.h"
#include "RefParser.h"
#include "RefExporter.h"
#include "EnglishName.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileInfo>
#include <QProgressDialog>

PagePapers::PagePapers(QWidget *parent)
	: Page(parent)
{
	currentRow = -1;
	currentPaperID = -1;
	setting = UserSetting::getInstance();

	ui.setupUi(this);
	ui.tvPapers->init("PagePapers");   // set the table name for the view

	onResetPapers();   // init model, table ...

	mapper.setModel(&modelPapers);
	mapper.addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper.addMapping(ui.teNote,     PAPER_NOTE);

	ui.tvPapers->setModel(&modelPapers);
	ui.tvPapers->hideColumn(PAPER_ID);
//    for(int col = PAPER_TYPE; col <= PAPER_NOTE; ++col)
//        ui.tvPapers->hideColumn(col);
	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);
	ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);

	ui.widgetWordCloud->setTableNames("Tags", "PaperTag", "Paper");

	loadGeometry();

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			&mapper, SLOT(setCurrentModelIndex(QModelIndex)));
    connect(ui.tvPapers->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection)));

	connect(ui.tvPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
			this, SLOT(onSubmitPaper()));
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
    connect(ui.widgetQuotes,     SIGNAL(doubleClicked(int)), this, SLOT(onQuoteDoubleClicked(int)));
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
    }
    emit selectionValid(!selected.isEmpty());
}

// only triggered by mouse click, not programmatically
void PagePapers::onClicked() {
	Navigator::getInstance()->addFootStep(this, currentPaperID);
	// TODO: move to onCurrentRowChanged?
}

void PagePapers::jumpToID(int id)
{
	if(id < 0)
		return;
	if(int row = idToRow(&modelPapers, PAPER_ID, id))
	{
		currentRow = row;
		ui.tvPapers->selectRow(currentRow);  // will trigger onCurrentRowChanged()
		ui.tvPapers->scrollTo(modelPapers.index(row, PAPER_TITLE));
		ui.tvPapers->setFocus();
	}
}

void PagePapers::add()
{
	reset();
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Add Paper"));
	if(dlg.exec() == QDialog::Accepted)
        insertReference(dlg.getReference());
}

void PagePapers::onEditPaper()
{
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Paper"));
    Reference oldRef = exportReference(currentRow);
    dlg.setReference(oldRef);

	if(dlg.exec() == QDialog::Accepted)
	{
        Reference newRef = dlg.getReference();
        updateReference(currentRow, newRef);
        onSubmitPaper();

        QString oldTitle = oldRef.getValue("title").toString();
        QString newTitle = newRef.getValue("title").toString();
        if(oldTitle != newTitle)
        {
            renameTitle(oldTitle, newTitle);
            reloadAttachments();           // refresh attached files after renaming
        }

        if(newRef.getValue("note") != oldRef.getValue("note"))   // changing note means reading
            setPaperRead();
	}
}

void PagePapers::insertReference(const Reference& ref)
{
    int lastRow = modelPapers.rowCount();
    modelPapers.insertRow(lastRow);
    currentPaperID = getNextID("Papers", "ID");
    modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), currentPaperID);

    updateReference(lastRow, ref);
    onBookmark(true);    // attach the ReadMe tag
}

void PagePapers::updateReference(int row, const Reference& ref)
{
    modelPapers.setData(modelPapers.index(row, PAPER_TITLE),       ref.getValue("title"));
    modelPapers.setData(modelPapers.index(row, PAPER_YEAR),        ref.getValue("year"));
    modelPapers.setData(modelPapers.index(row, PAPER_TYPE),        ref.getValue("type"));
    modelPapers.setData(modelPapers.index(row, PAPER_PUBLICATION), ref.getValue("publication"));
    modelPapers.setData(modelPapers.index(row, PAPER_ABSTRACT),    ref.getValue("abstract"));
    modelPapers.setData(modelPapers.index(row, PAPER_VOLUME),      ref.getValue("volume"));
    modelPapers.setData(modelPapers.index(row, PAPER_ISSUE),       ref.getValue("issue"));
    modelPapers.setData(modelPapers.index(row, PAPER_STARTPAGE),   ref.getValue("startpage"));
    modelPapers.setData(modelPapers.index(row, PAPER_ENDPAGE),     ref.getValue("endpage"));
    modelPapers.setData(modelPapers.index(row, PAPER_PUBLISHER),   ref.getValue("publisher"));
    modelPapers.setData(modelPapers.index(row, PAPER_EDITORS),     ref.getValue("editors"));
    modelPapers.setData(modelPapers.index(row, PAPER_ADDRESS),     ref.getValue("address"));
    modelPapers.setData(modelPapers.index(row, PAPER_URL),         ref.getValue("url"));
    modelPapers.setData(modelPapers.index(row, PAPER_NOTE),        ref.getValue("note"));
    modelPapers.setData(modelPapers.index(row, PAPER_AUTHORS),     ref.getValue("authors").toStringList().join("; "));
    updateTags(ref.getValue("tags").toStringList());
    onSubmitPaper();
}

void PagePapers::updateTags(const QStringList& tags)
{
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
		foreach(QModelIndex idx, idxList)     // find all selected indexes
            delPaper(rowToID(idx.row()));     // delete in the db
        modelPapers.select();                 // reload db
	}
}

int PagePapers::rowToID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}

void PagePapers::onImport()
{
    // get input files
    QString lastPath = setting->getLastImportPath();
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Import references", lastPath,
        "Reference (*.enw *.ris *.bib);;All files (*.*)");
    if(files.isEmpty())
        return;
    setting->setLastImportPath(QFileInfo(files.front()).absolutePath());

    foreach(QString filePath, files)
    {
        // find spec and parser
        QString extension = QFileInfo(filePath).suffix();
        IRefParser*    parser = ParserFactory::getInstance()->getParser(extension);
        RefFormatSpec* spec   = SpecFactory  ::getInstance()->getSpec(extension);
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
            continue;
        }

        // parse
        QList<Reference> references = parser->parse(file.readAll(), spec);
        if(references.isEmpty())
            continue;

        foreach(const Reference& ref, references)
        {
            QString title = ref.getValue("title").toString();
            if(paperExists(title)) {
                QMessageBox::warning(this, tr("Title exists"),
                                     tr("\"%1\" already exists in the database, and is skipped").arg(title));
            }
            else
            {
                insertReference(ref);   // currentPaperID will equals that of the newly added

                // add the pdf file with the same name
                QString pdfPath = QFileInfo(filePath).path() + "/" + QFileInfo(filePath).baseName() + ".pdf";
                if(QFile::exists(pdfPath))
                    addAttachment(currentPaperID, suggestAttachmentName(pdfPath), pdfPath);

                reloadAttachments();
            }
        }
    }
}

// submit the model
// keep selecting current paper
void PagePapers::onSubmitPaper()
{
	int backup = currentPaperID;
	if(!modelPapers.submitAll())
		QMessageBox::critical(this, tr("Error"), modelPapers.lastError().text());
	currentPaperID = backup;
    qApp->processEvents();
	jumpToID(backup);
}

void PagePapers::search(const QString& target)
{
	// filter papers
	modelPapers.setFilter(
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
		int paperID = modelPapers.data(idx).toInt();
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
		int paperID = modelPapers.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
		{
			ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", tag->text()), paperID);
			if(tag->text() == "ReadMe")   // ReadMe tag is removed
				reset();                  // unbold the title
		}
	}
	highLightTags();
}

void PagePapers:: onResetPapers()
{
	dropTempView();

	int backupID = currentPaperID;
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);
	modelPapers.setTable("Papers");
	modelPapers.select();
	while(modelPapers.canFetchMore())
		modelPapers.fetchMore();
	modelPapers.setHeaderData(PAPER_ATTACHED, Qt::Horizontal, "@");

	modelPapers.sort(PAPER_TITLE, Qt::AscendingOrder);
	// FIXME: sort by view does not work

	currentPaperID = backupID;
	jumpToID(currentPaperID);
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
		modelPapers.setFilter(
			tr("ID in (select Paper from PaperTag where Tag in (%1))").arg(tagIDs.join(",")));
	else
	{
		dropTempView();
		QSqlQuery query;    // create a temp table for selected tags id
		query.exec(tr("create view SelectedTags as select * from Tags where ID in (%1)")
									.arg(tagIDs.join(",")));

		// select papers that contain all the selected tags
		modelPapers.setFilter("not exists \
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
	ui.tabWidget->setCurrentWidget(ui.tabAttachments);
	ui.widgetAttachments->onAddFile();
}

void PagePapers::onReadPDF()
{
	openAttachment(currentPaperID, "Paper.pdf");
}

void PagePapers::onFullTextSearch(const QString& target)
{
	onResetPapers();
	if(target.isEmpty())
		return;

	int rowCount = modelPapers.rowCount();
	QProgressDialog progress(tr("Searching..."), tr("Abort"), 0, rowCount, this);
	progress.setWindowModality(Qt::WindowModal);

	QStringList filter;
	for(int row = 0; row < rowCount; ++row)
	{
		progress.setValue(row);
		if(fullTextSearch(rowToID(row), target))        // if found in this paper
			filter << tr("ID = %1").arg(rowToID(row));  // add the paper to filter
		if(progress.wasCanceled())
			break;
	}

	if(filter.isEmpty())
		QMessageBox::information(this, tr("Full text search"), tr("No such paper!"));
	else
		modelPapers.setFilter(filter.join(" OR "));        // filter out papers
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
	{
		attachNewTag("ReadMe");
		highLightTags();
	}
	else
		setPaperRead();
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
	jumpToID(paperID);
	Navigator::getInstance()->addFootStep(this, paperID);
}

void PagePapers::onQuoteDoubleClicked(int quoteID) {
	MainWindow::getInstance()->jumpToQuote(quoteID);
}

Reference PagePapers::exportReference(int row) const
{
    Reference ref;
    QSqlRecord record = modelPapers.record(row);
    for(int col = 0; col < record.count(); ++col)
    {
        QString fieldName = record.fieldName(col).toLower();
        QVariant fieldValue = record.value(col);
        if(fieldName == "authors")
            ref.setValue(fieldName, EnglishName::fromLineToList(fieldValue.toString()));
        else if(fieldName != "id" && fieldName != "attached")
            ref.setValue(fieldName, fieldValue);
    }

    ref.setValue("tags", getTagsOfPaper(rowToID(row)));  // NOTE: tags is not stored in the paper table
    return ref;
}

void PagePapers::onExport()
{
    // get selected rows
    QModelIndexList idxList = ui.tvPapers->selectionModel()->selectedRows();
    if(idxList.isEmpty())
        return;

    // get file name
    QString lastPath = setting->getLastImportPath();
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export reference"), lastPath,
                                                    "Reference (*.enw *.ris *.bib);;All files (*.*)");
    if(filePath.isEmpty())
        return;
    setting->setLastImportPath(QFileInfo(filePath).absolutePath());

    // find spec
    QString extension = QFileInfo(filePath).suffix().toLower();
    RefFormatSpec* spec = SpecFactory::getInstance()->getSpec(extension);
    if(spec == 0)
    {
        QMessageBox::critical(this, tr("Error"),
                              tr("Can not find the specification for this reference format!"));
        return;
    }

    QFile file(filePath);
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        IRefExporter* exporter = ExporterFactory::getInstance()->getExporter(extension);
        QTextStream os(&file);
        foreach(QModelIndex idx, idxList)    // save each row
        {
            Reference ref = exportReference(idx.row());
            os << exporter->toString(ref, *spec) << "\r\n";
        }
    }
}
