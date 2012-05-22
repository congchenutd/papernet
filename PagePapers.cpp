#include "PagePapers.h"
#include "Common.h"
#include "OptionDlg.h"
#include "PaperDlg.h"
#include "AddQuoteDlg.h"
#include "Importer.h"
#include "../EnglishName/EnglishName.h"
#include "Navigator.h"
#include "AddTagDlg.h"
#include "Thesaurus.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QSqlQuery>
#include <QMenu>
#include <QFileInfo>
#include <QSqlError>
#include <QProgressDialog>

PagePapers::PagePapers(QWidget *parent)
	: Page(parent)
{
	currentRow = -1;
	currentPaperID = -1;
	setting = MySetting<UserSetting>::getInstance();

	ui.setupUi(this);
	ui.tvPapers->init("PagePapers");   // set the table name for the view

	onResetPapers();   // init model, table ...

	mapper.setModel(&modelPapers);
	mapper.addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper.addMapping(ui.teNote,     PAPER_NOTE);

	ui.tvPapers->setModel(&modelPapers);
	ui.tvPapers->hideColumn(PAPER_ID);
	ui.tvPapers->hideColumn(PAPER_JOURNAL);
	ui.tvPapers->hideColumn(PAPER_ABSTRACT);
	ui.tvPapers->hideColumn(PAPER_NOTE);
	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);
	ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);
	sortByTitle();

	ui.widgetWordCloud->setTableNames("Tags", "PaperTag", "Paper");

	loadSplitterSizes();

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			&mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			this, SLOT(onCurrentRowChanged(QModelIndex)));
	connect(ui.tvPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
			this, SLOT(onSubmitPaper()));
	connect(ui.tvPapers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditPaper()));
	connect(ui.tvPapers, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
	connect(ui.tvPapers, SIGNAL(addQuote()),       this, SLOT(onAddQuote()));
	connect(ui.tvPapers, SIGNAL(printMe(bool)),    this, SLOT(onPrintMe(bool)));
	connect(ui.tvPapers, SIGNAL(readMe(bool)),     this, SLOT(onReadMe(bool)));

	connect(ui.widgetWordCloud, SIGNAL(filter(bool)), this, SLOT(onFilterPapers(bool)));
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),   this, SLOT(onResetPapers()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagToPaper()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPaper()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));

	connect(ui.relatedPapersWidget,    SIGNAL(doubleClicked(int)), this, SLOT(onRelatedPaperDoubleClicked(int)));
	connect(ui.coautheredPapersWidget, SIGNAL(doubleClicked(int)), this, SLOT(onRelatedPaperDoubleClicked(int)));
	connect(ui.quotesWidget,           SIGNAL(doubleClicked(int)), this, SLOT(onQuoteDoubleClicked(int)));
}

void PagePapers::onCurrentRowChanged(const QModelIndex& idx)
{
//	emit tableValid(idx.isValid());
	if(idx.isValid())
	{
		currentRow = idx.row();
		currentPaperID = getPaperID(idx.row());
        highLightTags();
        reloadAttachments();
        ui.relatedPapersWidget   ->setCentralPaper(currentPaperID);
        ui.coautheredPapersWidget->setCentralPaper(currentPaperID);
        ui.quotesWidget          ->setCentralPaper(currentPaperID);
	}
}

// only triggered by mouse click, not programmatically
void PagePapers::onClicked(const QModelIndex& idx) {
	Navigator::getInstance()->addFootStep(this, getPaperID(idx.row()));
}

void PagePapers::jumpToID(int id)
{
    int row = idToRow(&modelPapers, PAPER_ID, id);
	if(row > -1)
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
		insertRecord(dlg.getPaperRecord());
}

void PagePapers::onEditPaper()
{
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Paper"));

	PaperRecord record;
	QString oldTitle = modelPapers.data(modelPapers.index(currentRow, PAPER_TITLE)).toString();
	record.title = oldTitle;
	record.authors  = modelPapers.data(modelPapers.index(currentRow, PAPER_AUTHORS)) .toString();
	record.year     = modelPapers.data(modelPapers.index(currentRow, PAPER_YEAR))    .toInt();
	record.journal  = modelPapers.data(modelPapers.index(currentRow, PAPER_JOURNAL)) .toString();
	record.abstract = modelPapers.data(modelPapers.index(currentRow, PAPER_ABSTRACT)).toString();
	QString oldNote = modelPapers.data(modelPapers.index(currentRow, PAPER_NOTE))    .toString();
	record.note     = oldNote;
	record.tags     = getTagsOfPaper(currentPaperID);
	dlg.setPaperRecord(record);

	if(dlg.exec() == QDialog::Accepted)
	{
		updateRecord(currentRow, dlg.getPaperRecord());
		renameTitle(oldTitle, dlg.getTitle());
		reloadAttachments();           // refresh attached files after renaming

		// changing note -> reading
		if(dlg.getNote() != oldNote)
			setPaperRead();
	}
}

void PagePapers::insertRecord(const PaperRecord& record)
{
	int lastRow = modelPapers.rowCount();
	modelPapers.insertRow(lastRow);
	currentPaperID = getNextID("Papers", "ID");
	modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), currentPaperID);

	updateRecord(lastRow, record);
	onReadMe(true);    // attach the ReadMe tag
}

void PagePapers::updateRecord(int row, const PaperRecord& record)
{
	modelPapers.setData(modelPapers.index(row, PAPER_TITLE),    record.title);
	modelPapers.setData(modelPapers.index(row, PAPER_AUTHORS),  record.authors);
	modelPapers.setData(modelPapers.index(row, PAPER_YEAR),     record.year);
	modelPapers.setData(modelPapers.index(row, PAPER_JOURNAL),  record.journal);
	modelPapers.setData(modelPapers.index(row, PAPER_ABSTRACT), record.abstract);
	modelPapers.setData(modelPapers.index(row, PAPER_NOTE),     record.note);
	onSubmitPaper();
	updateTags(record.tags);
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
			delPaper(getPaperID(idx.row()));  // delete in the db
		modelPapers.select();                 // reload db
	}
}

int PagePapers::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}

void PagePapers::onImport()
{
	reset();
	QString lastPath = setting->getLastImportPath();
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Select one or more files to open", lastPath,
		"Reference (*.enw *.ris *.xml);;All files (*.*)");
	if(files.isEmpty())
		return;

	QString file = files.front();
	setting->setLastImportPath(QFileInfo(file).absolutePath());

	foreach(QString fileName, files)
	{
		Importer* importer = ImporterFactory::getImporter(fileName);
		if(!importer->import(fileName))
		{
			delete importer;
			continue;
		}

		QList<PaperRecord> results = importer->getResults();
		foreach(PaperRecord result, results)    // one file may have multiple records
		{
			int id = ::getPaperID(result.title);
			if(id > -1)   // merge to existing record
			{
				if(QMessageBox::warning(this, tr("Title exists"), tr("Do you want to merge into the existing record?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
					mergeRecord(idToRow(&modelPapers, PAPER_ID, id), result);
				}
			}
			else
			{
				insertRecord(result);

				// FIXME: this works only when there is one record
				// change it to EndNote(i)
				addAttachment(currentPaperID, suggestAttachmentName(fileName), fileName);

				// add the pdf file with the same name
				QString pdfFileName = QFileInfo(fileName).path() + "/" + QFileInfo(fileName).baseName() + ".pdf";
				if(QFile::exists(pdfFileName))
					addAttachment(currentPaperID, suggestAttachmentName(pdfFileName), pdfFileName);

				reloadAttachments();
			}
		}
	}
}

void PagePapers::mergeRecord(int row, const PaperRecord &record)
{
	QString title = modelPapers.data(modelPapers.index(row, PAPER_TITLE)).toString();
	if(!title.isEmpty())
		title += "; " + record.title;
	modelPapers.setData(modelPapers.index(row, PAPER_TITLE), title);

	QString authors = modelPapers.data(modelPapers.index(row, PAPER_AUTHORS)).toString();
	if(!authors.isEmpty())
		authors += "; " + record.authors;
	modelPapers.setData(modelPapers.index(row, PAPER_AUTHORS), authors);

	QString journal = modelPapers.data(modelPapers.index(row, PAPER_JOURNAL)).toString();
	if(!journal.isEmpty())
		journal += "; " + record.journal;
	modelPapers.setData(modelPapers.index(row, PAPER_JOURNAL), journal);

	modelPapers.setData(modelPapers.index(row, PAPER_YEAR), record.year);

	QString abstract = modelPapers.data(modelPapers.index(row, PAPER_ABSTRACT)).toString();
	if(!abstract.isEmpty())
		abstract += "\r\n\r\n" + record.abstract;
	modelPapers.setData(modelPapers.index(row, PAPER_ABSTRACT), abstract);

	modelPapers.submitAll();
}

// submit the model
// keep selecting current paper
void PagePapers::onSubmitPaper()
{
	int backup = currentPaperID;
	if(!modelPapers.submitAll())
		QMessageBox::critical(this, tr("Error"), modelPapers.lastError().text());
	currentPaperID = backup;
//	sortByTitle();
	jumpToID(backup);
}

void PagePapers::search(const QString& target)
{
	// filter papers
	modelPapers.setFilter(
				tr("Title    like \"%%1%\" or \
					Authors  like \"%%1%\" or \
					Year     like \"%%1%\" or \
					Journal  like \"%%1%\" or \
					Abstract like \"%%1%\" or \
					Note     like \"%%1%\" ").arg(target));

	// highlight tags
	ui.widgetWordCloud->search(target);
}

void PagePapers::onAddTag()
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
			if(tag->text() == "ReadMe")
				reset();    // unbold the title
		}
	}
	highLightTags();
}

void PagePapers:: onResetPapers()
{
	QSqlQuery query;
	query.exec(tr("drop view SelectedTags"));   // remove the temp table

	int backupID = currentPaperID;
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);
	modelPapers.setTable("Papers");
	modelPapers.select();
	while(modelPapers.canFetchMore())
		modelPapers.fetchMore();
	modelPapers.setHeaderData(PAPER_ATTACHED, Qt::Horizontal, "@");

	sortByTitle();
	currentPaperID = backupID;
	jumpToID(currentPaperID);
}

// filter papers with tags
void PagePapers::onFilterPapers(bool AND)
{
	dropTempView();

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

int PagePapers::getQuoteID(int row) const {
    return -1;
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
		if(fullTextSearch(getPaperID(row), target))        // if found in this paper
			filter << tr("ID = %1").arg(getPaperID(row));  // add the paper to filter
		if(progress.wasCanceled())
			break;
	}

	if(filter.isEmpty())
		QMessageBox::information(this, tr("Full text search"), tr("No such paper!"));
	else
		modelPapers.setFilter(filter.join(" OR "));        // filter out papers
}

void PagePapers::loadSplitterSizes()
{
	ui.splitterHorizontal->restoreState(setting->value("SplitterHorizontal").toByteArray());
	ui.splitterPapers    ->restoreState(setting->value("SplitterPapers")    .toByteArray());
}

void PagePapers::saveGeometry()
{
	ui.tvPapers->saveSectionSizes();  // sections

	// splitters
	setting->setValue("SplitterHorizontal", ui.splitterHorizontal->saveState());
	setting->setValue("SplitterPapers",     ui.splitterPapers->saveState());
}

void PagePapers::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		onResetPapers();
	else
		onFilterPapers();
}

void PagePapers::reloadAttachments() {
	ui.widgetAttachments->setPaper(currentPaperID);
}

void PagePapers::sortByTitle() {
	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
}

void PagePapers::onPrintMe(bool print)
{
	if(print)
		attachNewTag("PrintMe");
	else
		ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", "PrintMe"), currentPaperID);
	highLightTags();
}

void PagePapers::onReadMe(bool readMe)
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

void PagePapers::onRelatedPaperDoubleClicked(int paperID)
{
	jumpToID(paperID);
	Navigator::getInstance()->addFootStep(this, paperID);
}

void PagePapers::onQuoteDoubleClicked(int quoteID) {
	MainWindow::getInstance()->jumpToQuote(quoteID);
}
