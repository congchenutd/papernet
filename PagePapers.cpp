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
	thesaurus = new BigHugeThesaurus(this);

	ui.setupUi(this);
	ui.tvPapers->init("PagePapers");   // set the table name for the view

	onResetPapers();   // init model, table ...
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);

	mapper.setModel(&modelPapers);
	mapper.addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper.addMapping(ui.teNote,     PAPER_NOTE);

	ui.tvPapers->setModel(&modelPapers);
	ui.tvPapers->hideColumn(PAPER_ID);
	ui.tvPapers->hideColumn(PAPER_READ);
	ui.tvPapers->hideColumn(PAPER_JOURNAL);
	ui.tvPapers->hideColumn(PAPER_ABSTRACT);
	ui.tvPapers->hideColumn(PAPER_NOTE);
	ui.tvPapers->hideColumn(PAPER_PROXIMITY);
	ui.tvPapers->hideColumn(PAPER_COAUTHOR);
	ui.tvPapers->hideColumn(PAPER_ADDEDTIME);
	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);
	ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);
	sortByTitle();

	ui.widgetWordCloud->setTableNames("Tags", "PaperTag", "Paper");
	ui.tvQuotes->setModel(&modelQuotes);

	loadSplitterSizes();

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			&mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			this, SLOT(onCurrentRowChanged(QModelIndex)));
	connect(ui.tvPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
			this, SLOT(onSubmitPaper()));
	connect(ui.tvPapers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditPaper()));
	connect(ui.tvPapers, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
	connect(ui.tvPapers, SIGNAL(showRelated()),    this, SLOT(onShowRelated()));
	connect(ui.tvPapers, SIGNAL(showCoauthored()), this, SLOT(onShowCoauthored()));
	connect(ui.tvPapers, SIGNAL(addQuote()),       this, SLOT(onAddQuote()));
	connect(ui.tvQuotes, SIGNAL(addQuote()),       this, SLOT(onAddQuote()));
	connect(ui.tvQuotes, SIGNAL(delQuotes()),      this, SLOT(onDelQuotes()));
	connect(ui.tvQuotes, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditQuote(QModelIndex)));

	connect(ui.widgetWordCloud, SIGNAL(filter(bool)), this, SLOT(onFilterPapers(bool)));
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),   this, SLOT(onResetPapers()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagToPaper()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPaper()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));

	connect(thesaurus, SIGNAL(response(QStringList)), this, SLOT(onThesaurus(QStringList)));

}

void PagePapers::onCurrentRowChanged(const QModelIndex& idx)
{
//	emit tableValid(idx.isValid());
	if(idx.isValid())
	{
		currentRow = idx.row();
		currentPaperID = getPaperID(idx.row());
		highLightTags();
		updateQuotes();
		reloadAttachments();
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
		ui.tvPapers->setFocus();
		ui.tvPapers->scrollTo(modelPapers.index(row, PAPER_TITLE));
	}
}

void PagePapers::add()
{
	reset();
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Add Paper"));
	if(dlg.exec() == QDialog::Accepted)
	{
		int lastRow = modelPapers.rowCount();
		modelPapers.insertRow(lastRow);
		currentPaperID = getNextID("Papers", "ID");
		modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), currentPaperID);
		updateRecord(lastRow, dlg);
		onSubmitPaper();
		updateTags(dlg.getTags());
	}
}

void PagePapers::onEditPaper()
{
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Paper"));
	QString oldTitle = modelPapers.data(modelPapers.index(currentRow, PAPER_TITLE)).toString();
	dlg.setTitle(oldTitle);
	dlg.setAuthors (modelPapers.data(modelPapers.index(currentRow, PAPER_AUTHORS)) .toString());
	dlg.setYear    (modelPapers.data(modelPapers.index(currentRow, PAPER_YEAR))    .toInt());
	dlg.setJournal (modelPapers.data(modelPapers.index(currentRow, PAPER_JOURNAL)) .toString());
	dlg.setAbstract(modelPapers.data(modelPapers.index(currentRow, PAPER_ABSTRACT)).toString());
	dlg.setNote    (modelPapers.data(modelPapers.index(currentRow, PAPER_NOTE))    .toString());
	dlg.setTags    (getTagsOfPaper(currentPaperID));
	if(dlg.exec() != QDialog::Accepted)
		return;

	updateRecord(currentRow, dlg);
	onSubmitPaper();
	renameTitle(oldTitle, dlg.getTitle());
	reloadAttachments();           // refresh attached files after renaming
	if(!dlg.getNote().isEmpty())   // paper with notes indicates its been read
		setPaperRead(currentPaperID);
	updateTags(dlg.getTags());
}

void PagePapers::updateRecord(int row, const PaperDlg& dlg)
{
	modelPapers.setData(modelPapers.index(row, PAPER_TITLE),    dlg.getTitle());
	modelPapers.setData(modelPapers.index(row, PAPER_AUTHORS),  dlg.getAuthors());
	modelPapers.setData(modelPapers.index(row, PAPER_YEAR),     dlg.getYear());
	modelPapers.setData(modelPapers.index(row, PAPER_JOURNAL),  dlg.getJournal());
	modelPapers.setData(modelPapers.index(row, PAPER_ABSTRACT), dlg.getAbstract());
	modelPapers.setData(modelPapers.index(row, PAPER_NOTE),     dlg.getNote());
}

void PagePapers::updateTags(const QStringList& tags)
{
	// remove all connections to tags
	QSqlQuery query;
	query.exec(tr("delete from PaperTag where Paper = %1").arg(currentPaperID));

	// add connections back
	foreach(QString tag, tags)
	{
		int tagID = getTagID("Tags", tag);
		if(tagID < 0)   // new tag
		{
			tagID = getNextID("Tags", "ID");
			ui.widgetWordCloud->addTag(tagID, tag);
		}
		ui.widgetWordCloud->addTagToItem(tagID, currentPaperID);
	}
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

	// Can't remember why these two lines are here
	//onSubmitPaper();
	//ui.tvPapers->sortByColumn(PAPER_ID, Qt::AscendingOrder);

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

		QList<ImportResult> results = importer->getResults();
		foreach(ImportResult result, results)    // one file may have multiple records
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
				QString pdfFileName = QFileInfo(fileName).path() + QFileInfo(fileName).baseName() + ".pdf";
				if(QFile::exists(pdfFileName))
					addAttachment(currentPaperID, suggestAttachmentName(pdfFileName), pdfFileName);
			}
		}
	}

	onSubmitPaper();
}

void PagePapers::insertRecord(const ImportResult &record)
{
	int lastRow = modelPapers.rowCount();
	modelPapers.insertRow(lastRow);
	currentPaperID = getNextID("Papers", "ID");
	modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), currentPaperID);
	modelPapers.setData(modelPapers.index(lastRow, PAPER_TITLE),    record.title);
	modelPapers.setData(modelPapers.index(lastRow, PAPER_AUTHORS),  record.authors);
	modelPapers.setData(modelPapers.index(lastRow, PAPER_JOURNAL),  record.journal);
	modelPapers.setData(modelPapers.index(lastRow, PAPER_YEAR),     record.year);
	modelPapers.setData(modelPapers.index(lastRow, PAPER_ABSTRACT), record.abstract);
	modelPapers.submitAll();
}

void PagePapers::mergeRecord(int row, const ImportResult &record)
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
	hideColoring();
	int backup = currentPaperID;
	if(!modelPapers.submitAll())
		QMessageBox::critical(this, tr("Error"), tr("Database submission failed!"));
	currentPaperID = backup;
//	sortByTitle();
	jumpToID(backup);
}

void PagePapers::search(const QString& target) {
	if(!target.isEmpty())
		modelPapers.setFilter(
					tr("Title    like \"%%1%\" or \
						Authors  like \"%%1%\" or \
						Year     like \"%%1%\" or \
						Journal  like \"%%1%\" or \
						Abstract like \"%%1%\" or \
						Note     like \"%%1%\" ").arg(target));
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
void PagePapers::highLightTags()
{
	ui.widgetWordCloud->unselectAll();
	ui.widgetWordCloud->unrelateAll();
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
			ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", tag->text()), paperID);
	}
	highLightTags();
}

void PagePapers::onResetPapers()
{
	hideColoring();
	modelPapers.setTable("Papers");
	modelPapers.select();
	while(modelPapers.canFetchMore())
		modelPapers.fetchMore();
	modelPapers.setHeaderData(PAPER_READ,     Qt::Horizontal, "R");
	modelPapers.setHeaderData(PAPER_ATTACHED, Qt::Horizontal, "@");

	sortByTitle();
	jumpToID(currentPaperID);
}

// filter papers with tags
void PagePapers::onFilterPapers(bool AND)
{
	hideColoring();
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

		query.exec(tr("drop view SelectedTags"));   // remove the temp table
	}
}

void PagePapers::onShowRelated()
{
	onResetPapers();
	QSqlDatabase::database().transaction();

	// gather related tags: tags this phrase has (direct), and their proximate tags (from tagThesaurus)
	QSqlQuery query;    // query direct tags' names
	query.exec(tr("select Tags.Name from Tags, PaperTag \
				   where PaperTag.Paper = %1 and PaperTag.Tag = Tags.ID").arg(currentPaperID));
	while(query.next())
		thesaurus->request(query.value(0).toString());   // query proximate tags with this direct tag

	// calculate proximity by direct tags:
	// 1. find in PaperTag all direct tags
	// 2. count the # of all other papers that have these tags
	// 3. associate the # with the papers
	query.exec(tr("select Papers.ID, count(Paper) Proximity from Papers, PaperTag \
				   where Tag in (select Tag from PaperTag where Paper = %1) \
						 and PaperTag.Paper != %1 and Papers.ID = PaperTag.Paper \
				   group by PaperTag.Paper").arg(currentPaperID));

	// save proximity
	QSqlQuery subQuery;
	while(query.next()) {
		subQuery.exec(tr("update Papers set Proximity = %2 where ID = %1")
			.arg(query.value(0).toInt())
			.arg(query.value(1).toInt()));
	}

	// set itself the max proximity
	query.exec(tr("update Papers set Proximity = \
					  (select max(Proximity)+1 from Papers where ID <> %1) \
				  where ID = %1").arg(currentPaperID));
	QSqlDatabase::database().commit();

	sortByProximity();
	jumpToID(currentPaperID);    // keep highlighting
}

void PagePapers::onThesaurus(const QStringList& relatedTags)
{
	if(relatedTags.isEmpty())
		return;

	QSqlDatabase::database().transaction();

	// get the IDs of relatedTags
	QSqlQuery query;
	QStringList tagIDs;
	foreach(QString tagName, relatedTags)
	{
		query.prepare("select ID from Tags where Name = :tagName ");
		query.bindValue(":tagName", tagName);
		query.exec();
		while(query.next())
			tagIDs << query.value(0).toString();
	}

	// calculate proximity by proximate tags:
	// 1. count the # of all other papers that have these tags
	// 2. associate the # with the papers
	query.exec(tr("select Papers.ID, count(Paper) Proximity from Papers, PaperTag \
				   where Tag in (%1) \
						 and PaperTag.Paper != %2 and Papers.ID = PaperTag.Paper \
				   group by PaperTag.Paper").arg(tagIDs.join(",")).arg(currentPaperID));

	// update proximity
	QSqlQuery subQuery;
	while(query.next()) {
		subQuery.exec(tr("update Papers set Proximity = \
							 (select Proximity from Papers where ID = %1) + %2 \
						 where ID = %1")
			.arg(query.value(0).toInt())
			.arg(query.value(1).toInt()));
	}

	// give itself the max proximity
	query.exec(tr("update Papers set Proximity = \
					  (select max(Proximity)+1 from Papers where ID <> %1) \
				  where ID = %1").arg(currentPaperID));

	QSqlDatabase::database().commit();

	sortByProximity();
	jumpToID(currentPaperID);   // keep highlighting
}

void PagePapers::onShowCoauthored()
{
	onResetPapers();
	QSqlDatabase::database().transaction();

	QStringList authors = modelPapers.data(
		modelPapers.index(currentRow, PAPER_AUTHORS)).toString().split(";");
	foreach(QString author, authors)
		for(int row=0; row<modelPapers.rowCount(); ++row)
		{
			QStringList names = modelPapers.data(modelPapers.index(row, PAPER_AUTHORS)).toString().split(";");
			foreach(QString name, names)
				if(EnglishName::compare(name, author))
				{
					int coauthor = modelPapers.data(modelPapers.index(row, PAPER_COAUTHOR)).toInt();
					modelPapers.setData(modelPapers.index(row, PAPER_COAUTHOR), coauthor+1);
				}
		}
	modelPapers.submitAll();
	QSqlDatabase::database().commit();

	ui.tvPapers->sortByColumn(PAPER_COAUTHOR, Qt::DescendingOrder);  // sort by author
	jumpToID(currentPaperID);
}

void PagePapers::onAddQuote()
{
	AddQuoteDlg* dlg = new AddQuoteDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onResetPapers()));
	dlg->setWindowTitle(tr("Add Quote"));
	dlg->setQuoteID(getNextID("Quotes", "ID"));    // create new quote id
	dlg->addRef(getPaperTitle(currentPaperID));    // add itself
	dlg->show();    // not modal, s.t. can switch between paper and quote pages
}

void PagePapers::updateQuotes()
{
	modelQuotes.setQuery(tr("select Title, Quote from Quotes where id in \
							  (select Quote from PaperQuote where Paper = %1) order by Title")
							  .arg(currentPaperID));
	ui.tvQuotes->resizeColumnToContents(0);
}

void PagePapers::onEditQuote(const QModelIndex& idx)
{
	AddQuoteDlg* dlg = new AddQuoteDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onResetPapers()));
	dlg->setWindowTitle(tr("Edit Quote"));
	dlg->setQuoteID(getQuoteID(idx.row()));
	dlg->show();    // not modal, s.t. can switch between paper and quote pages
}

void PagePapers::onDelQuotes()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tvQuotes->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delQuote(getQuoteID(idx.row()));
		onResetPapers();
	}
}

int PagePapers::getQuoteID(int row) const {
	return ::getQuoteID(modelQuotes.data(modelQuotes.index(row, 0)).toString());
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
	ui.splitterDetails   ->restoreState(setting->value("SplitterDetails")   .toByteArray());
}

void PagePapers::saveGeometry()
{
	ui.tvPapers->saveSectionSizes();  // sections

	// splitters
	setting->setValue("SplitterHorizontal", ui.splitterHorizontal->saveState());
	setting->setValue("SplitterPapers",     ui.splitterPapers->saveState());
	setting->setValue("SplitterDetails",    ui.splitterDetails->saveState());
}

void PagePapers::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		onResetPapers();
	else
		onFilterPapers();
}

void PagePapers::hideColoring()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Coauthor = 0"));
	query.exec(tr("update Papers set Proximity = 0"));
	QSqlDatabase::database().commit();
}

void PagePapers::reloadAttachments() {
	ui.widgetAttachments->setPaper(currentPaperID);
}

void PagePapers::sortByTitle() {
	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
}

void PagePapers::sortByProximity() {
	ui.tvPapers->sortByColumn(PAPER_PROXIMITY, Qt::DescendingOrder);
}
