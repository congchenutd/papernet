#include "PagePapers.h"
#include "Common.h"
#include "OptionDlg.h"
#include "PaperDlg.h"
#include "AddQuoteDlg.h"
#include "Importer.h"
#include "../EnglishName/EnglishName.h"
#include "Navigator.h"
#include "AddTagDlg.h"
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
	setting = MySetting<UserSetting>::getInstance();

	ui.setupUi(this);
	ui.tvPapers->init("PagePapers");   // set the table name for the view

	onResetPapers();
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);

	mapper.setModel(&modelPapers);
	mapper.addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper.addMapping(ui.teNote,     PAPER_NOTE);

	ui.tvPapers->setModel(&modelPapers);
	ui.tvPapers->hideColumn(PAPER_ID);
	ui.tvPapers->hideColumn(PAPER_READ);
	ui.tvPapers->hideColumn(PAPER_TAGGED);
	ui.tvPapers->hideColumn(PAPER_JOURNAL);
	ui.tvPapers->hideColumn(PAPER_ABSTRACT);
	ui.tvPapers->hideColumn(PAPER_NOTE);
	ui.tvPapers->hideColumn(PAPER_PROXIMITY);
	ui.tvPapers->hideColumn(PAPER_COAUTHOR);
	ui.tvPapers->hideColumn(PAPER_ADDEDTIME);
	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);
	ui.tvPapers->setColumnWidth(PAPER_TAGGED,   32);
	ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);
	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);

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

	connect(ui.widgetWordCloud, SIGNAL(filter()),    this, SLOT(onFilterPapers()));
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),  this, SLOT(onResetPapers()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),    this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),    this, SLOT(onAddTagToPaper()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()), this, SLOT(onDelTagFromPaper()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));
}

void PagePapers::onCurrentRowChanged(const QModelIndex& idx)
{
	emit tableValid(idx.isValid());
	if(idx.isValid())
		selectRow(idx.row());
}

// only triggered by mouse click, not programmatically
void PagePapers::onClicked(const QModelIndex& idx)
{
	selectRow(idx.row());
	Navigator::getInstance()->addFootStep(this, currentPaperID);
}

void PagePapers::selectRow(int row)
{
	currentRow = row;
	currentPaperID = getPaperID(row);
	highLightTags();
	updateQuotes();
	ui.widgetAttachments->setPaper(currentPaperID);
}

void PagePapers::jumpToID(int id)
{
	int row = idToRow(&modelPapers, PAPER_ID, id);
	if(row > -1)
	{
		currentRow = row;
		ui.tvPapers->selectRow(currentRow);  // will trigger onCurrentRowChanged()
		ui.tvPapers->setFocus();
	}
}

void PagePapers::add()
{
	onSubmitPaper();
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
	if(!dlg.getNote().isEmpty())   // paper with notes indicates being read
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
	QSqlQuery query;
	query.exec(tr("delete from PaperTag where Paper = %1").arg(currentPaperID));
	foreach(QString tag, tags)
	{
		int tagID = getTagID("Tags", tag);
		if(tagID < 0)
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
		foreach(QModelIndex idx, idxList)
			delPaper(getPaperID(idx.row()));
		modelPapers.select();
	}
}

int PagePapers::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}

void PagePapers::onImport()
{
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
				addAttachment(currentPaperID, "EndNote." + QFileInfo(fileName).suffix(), fileName);
			}
		}
	}

	onSubmitPaper();
//	onResetPapers();   // can't remember why
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

void PagePapers::onSubmitPaper()
{
	hideRelated();
	hideCoauthor();
	int backup = currentPaperID;
	if(!modelPapers.submitAll())
		QMessageBox::critical(this, tr("Error"), tr("Database submission failed!"));
	currentPaperID = backup;
	jumpToID(backup);
}

PagePapers::~PagePapers() {
	//onSubmitPaper();   // everything should have been manually submitted
}

void PagePapers::search(const QString& target)
{
	if(target.isEmpty())
		onResetPapers();
	else
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
	if(dlg.exec() == QDialog::Accepted && !dlg.getText().isEmpty())
	{
		int tagID = getNextID("Tags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());
		ui.widgetWordCloud->addTagToItem(tagID, currentPaperID);
		highLightTags();
	}
}

void PagePapers::onAddTagToPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
		int paperID = modelPapers.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->addTagToItem(getTagID("Tags", tag->text()),
											 paperID);
	}
	highLightTags();
}

void PagePapers::highLightTags()
{
	ui.widgetWordCloud->unselectAll();
	ui.widgetWordCloud->highLight(getTagsOfPaper(currentPaperID));
}

void PagePapers::onDelTagFromPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
		int paperID = modelPapers.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->removeTagFromItem(getTagID("Tags", tag->text()),
												  paperID);
	}
	highLightTags();
}

void PagePapers::onResetPapers()
{
	modelPapers.setTable("Papers");
	modelPapers.select();
	while(modelPapers.canFetchMore())
		modelPapers.fetchMore();
	modelPapers.setHeaderData(PAPER_READ,     Qt::Horizontal, "R");
	modelPapers.setHeaderData(PAPER_TAGGED,   Qt::Horizontal, "!");
	modelPapers.setHeaderData(PAPER_ATTACHED, Qt::Horizontal, "@");

	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
	hideRelated();
	hideCoauthor();
	jumpToID(currentPaperID);
}

void PagePapers::onFilterPapers()
{
	hideRelated();
	hideCoauthor();
	QStringList tagClauses;
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)
		tagClauses << tr("Tag = %1").arg(getTagID("Tags", tag->text()));
	modelPapers.setFilter(tr("ID in (select Paper from PaperTag where %1)")
								.arg(tagClauses.join(" OR ")));
}

void PagePapers::onShowRelated()
{
	hideCoauthor();
	QSqlDatabase::database().transaction();
	QSqlQuery query, subQuery;
	query.exec(tr("update Papers set Proximity = 0"));
	query.exec(tr("select Papers.ID, count(Paper) Proximity from Papers, PaperTag \
				  where Tag in (select Tag from PaperTag where Paper = %1) \
				  and Paper != %1 and ID = Paper \
				  group by Paper").arg(currentPaperID));

	while(query.next()) {
		subQuery.exec(tr("update Papers set Proximity = %1 where ID = %2")
			.arg(query.value(1).toInt())
			.arg(query.value(0).toInt()));
	}

	query.exec(tr("update Papers set Proximity = (select max(Proximity)+1 from Papers) \
				  where ID = %1").arg(currentPaperID));
	QSqlDatabase::database().commit();

	ui.tvPapers->sortByColumn(PAPER_PROXIMITY, Qt::DescendingOrder);
	jumpToID(currentPaperID);
}

void PagePapers::hideRelated()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Proximity = 0"));
	QSqlDatabase::database().commit();
}

void PagePapers::onShowCoauthored()
{
	hideRelated();
	hideCoauthor();

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

	ui.tvPapers->sortByColumn(PAPER_COAUTHOR, Qt::DescendingOrder);
	jumpToID(currentPaperID);
}

void PagePapers::hideCoauthor()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Coauthor = 0"));
	QSqlDatabase::database().commit();
}

void PagePapers::onAddQuote()
{
	AddQuoteDlg* dlg = new AddQuoteDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onResetPapers()));
	dlg->setWindowTitle(tr("Add Quote"));
	dlg->setQuoteID(getNextID("Quotes", "ID"));
	dlg->addRef(getPaperTitle(currentPaperID));
	dlg->show();
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
	dlg->show();
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

void PagePapers::jumpToPaper(const QString& title) {
	jumpToID(::getPaperID(title));
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
		if(fullTextSearch(getPaperID(row), target))
			filter << tr("ID = %1").arg(getPaperID(row));
		if (progress.wasCanceled())
			break;
	}

	if(filter.isEmpty())
		QMessageBox::information(this, tr("Full text search"), tr("No such paper!"));
	else
		modelPapers.setFilter(filter.join(" OR "));
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