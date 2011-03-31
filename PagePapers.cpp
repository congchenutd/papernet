#include "PagePapers.h"
#include "Common.h"
#include "OptionDlg.h"
#include "PaperDlg.h"
#include "AddSnippetDlg.h"
#include "Importer.h"
#include "../EnglishName/EnglishName.h"
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
	: QWidget(parent)
{
	currentRowPapers = -1;
	currentRowTags   = -1;
	setting = MySetting<UserSetting>::getInstance();

	ui.setupUi(this);
	ui.tvPapers->init("PagePapers");

	onResetPapers();
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);
	resetAllTags();
	modelAllTags.setEditStrategy(QSqlTableModel::OnManualSubmit);

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
	ui.tvPapers->setColumnWidth(PAPER_TAGGED,   32);
	ui.tvPapers->setColumnWidth(PAPER_ATTACHED, 32);
	ui.tvPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);

	ui.lvAllTags->setModel(&modelAllTags);
	ui.lvAllTags->setModelColumn(TAG_NAME);
	ui.lvTags->setModel(&modelTags);

	ui.tvSnippets->setModel(&modelSnippets);

	loadSplitterSizes();

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			&mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowPapersChanged(QModelIndex)));
	connect(ui.tvPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
			this, SLOT(onSubmitPaper()));
	connect(ui.tvPapers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditPaper()));
	connect(ui.tvPapers, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));

	connect(ui.lvAllTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowAllTagsChanged()));
	connect(ui.btAddTag,  SIGNAL(clicked()), this, SLOT(onAddTag()));
	connect(ui.lvAllTags, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditTag()));
	connect(ui.btDelTag,  SIGNAL(clicked()), this, SLOT(onDelTag()));
	connect(ui.btAddTagToPaper,   SIGNAL(clicked()), this, SLOT(onAddTagToPaper()));
	connect(ui.btDelTagFromPaper, SIGNAL(clicked()), this, SLOT(onDelTagFromPaper()));
	connect(ui.btFilter, SIGNAL(clicked(bool)), this, SLOT(onFilter(bool)));

	connect(ui.lvTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowTagsChanged()));

	connect(ui.tvPapers, SIGNAL(showRelated()),    this, SLOT(onShowRelated()));
	connect(ui.tvPapers, SIGNAL(showCoauthored()), this, SLOT(onShowCoauthored()));
	connect(ui.tvPapers,   SIGNAL(addSnippet()),   this, SLOT(onAddSnippet()));
	connect(ui.tvSnippets, SIGNAL(addSnippet()),   this, SLOT(onAddSnippet()));
	connect(ui.tvSnippets, SIGNAL(delSnippets()),  this, SLOT(onDelSnippets()));
	connect(ui.tvSnippets, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditSnippet(QModelIndex)));
}

void PagePapers::onCurrentRowPapersChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	ui.btDelTag->setEnabled(valid);
	emit tableValid(valid);

	if(valid)
		onClicked(idx);
}

void PagePapers::onAddPaper()
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
		modelPapers.setData(modelPapers.index(lastRow, PAPER_TITLE),    dlg.getTitle());
		modelPapers.setData(modelPapers.index(lastRow, PAPER_AUTHORS),  dlg.getAuthors());
		modelPapers.setData(modelPapers.index(lastRow, PAPER_YEAR),     dlg.getYear());
		modelPapers.setData(modelPapers.index(lastRow, PAPER_JOURNAL),  dlg.getJournal());
		modelPapers.setData(modelPapers.index(lastRow, PAPER_ABSTRACT), dlg.getAbstract());
		modelPapers.setData(modelPapers.index(lastRow, PAPER_NOTE),     dlg.getNote());
		onSubmitPaper();
	}
}

void PagePapers::onEditPaper()
{
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Paper"));
	QString oldTitle = modelPapers.data(modelPapers.index(currentRowPapers, PAPER_TITLE)).toString();
	dlg.setTitle(oldTitle);
	dlg.setAuthors (modelPapers.data(modelPapers.index(currentRowPapers, PAPER_AUTHORS)) .toString());
	dlg.setYear    (modelPapers.data(modelPapers.index(currentRowPapers, PAPER_YEAR))    .toInt());
	dlg.setJournal (modelPapers.data(modelPapers.index(currentRowPapers, PAPER_JOURNAL)) .toString());
	dlg.setAbstract(modelPapers.data(modelPapers.index(currentRowPapers, PAPER_ABSTRACT)).toString());
	dlg.setNote    (modelPapers.data(modelPapers.index(currentRowPapers, PAPER_NOTE))    .toString());
	if(dlg.exec() == QDialog::Accepted)
	{
		renameTitle(oldTitle, dlg.getTitle());
		modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_TITLE),    dlg.getTitle());
		modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_AUTHORS),  dlg.getAuthors());
		modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_YEAR),     dlg.getYear());
		modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_JOURNAL),  dlg.getJournal());
		modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_ABSTRACT), dlg.getAbstract());
		modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_NOTE),     dlg.getNote());
		if(!dlg.getNote().isEmpty())
			setPaperRead(currentPaperID);
		onSubmitPaper();
	}
}

void PagePapers::onDelPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tvPapers->selectionModel()->selectedRows();
		QSqlDatabase::database().transaction();
		foreach(QModelIndex idx, idxList)
			delPaper(getPaperID(idx.row()));
		QSqlDatabase::database().commit();
		modelPapers.select();
	}
}

int PagePapers::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}
int PagePapers::getTagID(int row) const {
	return row > -1 ? modelTags.data(modelTags.index(row, TAG_ID)).toInt() : -1;
}
int PagePapers::getAllTagID(int row) const {
	return row > -1 ? modelAllTags.data(modelAllTags.index(row, TAG_ID)).toInt() : -1;
}

void PagePapers::selectID(int id)
{
	int row = idToRow(id);
	if(row > -1)
	{
		currentRowPapers = row;
		ui.tvPapers->selectRow(currentRowPapers);
		ui.tvPapers->setFocus();
	}
}

int PagePapers::idToRow(int id) const
{
	QModelIndexList indexes = modelPapers.match(
		modelPapers.index(0, PAPER_ID), Qt::DisplayRole, id, 1, Qt::MatchExactly | Qt::MatchWrap);
	return !indexes.isEmpty() ? indexes.at(0).row() : -1;
}

void PagePapers::onImport()
{
	QString lastPath = setting->getLastImportPath();
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Select one or more files to open", lastPath,
		"Reference (*.enw *.ris *.xml);;All files (*.*)");
	if(files.isEmpty())
		return;

	onSubmitPaper();
	ui.tvPapers->sortByColumn(PAPER_ID, Qt::AscendingOrder);

	QString file = files.front();
	setting->setLastImportPath(QFileInfo(file).absolutePath());

	foreach(QString fileName, files)
	{
		Importer* importer = ImporterFactory::getImporter(fileName);
		if(importer->import(fileName))
		{
            QList<ImportResult> results = importer->getResults();  // one file may have multiple records
            foreach(ImportResult result, results)
            {
                int id = ::getPaperID(result.title);
                if(id > -1)
                {
                    if(QMessageBox::warning(this, tr("Title exists"), tr("Do you want to merge into the existing record?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                        mergeRecord(idToRow(id), result);
                    }
                }
                else
				{
                    insertRecord(result);

					// Fix it: this works only when there is one record
					addAttachment(currentPaperID, "EndNote." + QFileInfo(fileName).suffix(), fileName);
                }
            }
		}
		delete importer;
	}

	onSubmitPaper();
	onResetPapers();
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
//	QString err = modelPapers.lastError().text();
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
	modelPapers.submitAll();
	currentPaperID = backup;
	selectID(backup);
}

PagePapers::~PagePapers() {
	onSubmitPaper();
}

void PagePapers::onSearch(const QString& target)
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

void PagePapers::onCurrentRowAllTagsChanged()
{
	QModelIndexList idxList = ui.lvAllTags->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRowTags = valid ? idxList.front().row() : -1;	
	ui.btDelTag ->setEnabled(valid);
	ui.btAddTagToPaper->setEnabled(valid && !isFiltered());

	if(isFiltered())  
	{
		if(currentRowTags < 0)
			return onResetPapers();   // no selected tags, show all papers
		filterPapers();
	}
}

void PagePapers::onAddTag()
{
	bool ok;
	QString tagName = QInputDialog::getText(this, "Add Tag", "Tag Name", 
		QLineEdit::Normal,	"New Tag", &ok);
	if(!ok || tagName.isEmpty())
		return;
	int lastRow = modelAllTags.rowCount();
	modelAllTags.insertRow(lastRow);
	int tagID = getNextID("Tags", "ID");
	modelAllTags.setData(modelAllTags.index(lastRow, TAG_ID),   tagID);
	modelAllTags.setData(modelAllTags.index(lastRow, TAG_NAME), tagName);
	modelAllTags.submitAll();

	// automatically add this tag to current paper
	addPaperTag(currentPaperID, tagID);
}

void PagePapers::onDelTag()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.lvAllTags->selectionModel()->selectedRows();
		QSqlDatabase::database().transaction();
		foreach(QModelIndex idx, idxList)
			delTag(getAllTagID(idx.row()));
		QSqlDatabase::database().commit();
		modelAllTags.select();
	}
}

void PagePapers::onEditTag()
{
	QString tag = modelAllTags.data(modelAllTags.index(currentRowTags, TAG_NAME)).toString();
	bool ok;
	tag = QInputDialog::getText(this, "Edit Tag", "Tag Name", QLineEdit::Normal, tag, &ok);
	if(!ok || tag.isEmpty())
		return;
	modelAllTags.setData(modelAllTags.index(currentRowTags, TAG_NAME), tag);
	modelAllTags.submitAll();
	updateTags();
}

void PagePapers::onAddTagToPaper()
{
	QModelIndexList idxList = ui.lvAllTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		addPaperTag(currentPaperID, getAllTagID(idx.row()));
	updateTags();
}

void PagePapers::updateTags()
{
	QString thisPapersTags(tr("(select Tag from PaperTag where Paper = %1)")
																.arg(currentPaperID));
	modelTags.setQuery(tr("select ID, Name from Tags where ID in %1 order by Name")
																.arg(thisPapersTags));
	ui.lvTags->setModelColumn(TAG_NAME);

	if(!isFiltered())
		modelAllTags.setFilter(tr("ID not in %1 order by Name").arg(thisPapersTags));
}

void PagePapers::onCurrentRowTagsChanged() {
	ui.btDelTagFromPaper->setEnabled(!ui.lvTags->selectionModel()->selectedRows().isEmpty());
}

void PagePapers::onDelTagFromPaper()
{
	QModelIndexList idxList = ui.lvTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		delPaperTag(currentPaperID, getTagID(idx.row()));
	updateTags();
}

void PagePapers::filterPapers()
{
	hideRelated();
	hideCoauthor();
	QStringList tagClauses;
	QModelIndexList idxList = ui.lvAllTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		tagClauses << tr("Tag = %1").arg(getAllTagID(idx.row()));
	if(tagClauses.isEmpty())
		tagClauses << tr("Tag = %1").arg(getAllTagID(currentRowTags));
	modelPapers.setFilter(tr("ID in (select Paper from PaperTag where %1)")
										.arg(tagClauses.join(" OR ")));
}

void PagePapers::onFilter(bool enabled)
{
	if(enabled)
		resetAllTags();   // show all tags
	else
		onResetPapers();    // show all papers
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
	selectID(currentPaperID);
}

void PagePapers::resetAllTags()
{
	modelAllTags.setTable("Tags");
	modelAllTags.setFilter("Name != \"\" order by Name");
	modelAllTags.select();
}

bool PagePapers::isFiltered() const {
	return ui.btFilter->isChecked();
}

void PagePapers::onClicked(const QModelIndex& idx)
{
	currentRowPapers = idx.row();
	currentPaperID = getPaperID(currentRowPapers);
	updateTags();
	ui.widgetAttachments->setPaper(currentPaperID);
	updateSnippets();
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
	selectID(currentPaperID);
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
		modelPapers.index(currentRowPapers, PAPER_AUTHORS)).toString().split(";");
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
	selectID(currentPaperID);
}

void PagePapers::hideCoauthor()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Coauthor = 0"));
	QSqlDatabase::database().commit();
}

void PagePapers::onAddSnippet()
{
	AddSnippetDlg* dlg = new AddSnippetDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onResetPapers()));
	dlg->setWindowTitle(tr("Add Snippet"));
	dlg->setSnippetID(getNextID("Snippets", "ID"));
	dlg->addPaper(getPaperTitle(currentPaperID));
	dlg->show();
}

void PagePapers::updateSnippets()
{
	modelSnippets.setQuery(tr("select Title, Snippet from Snippets where id in \
							  (select Snippet from PaperSnippet where Paper = %1) order by Title")
							  .arg(currentPaperID));
	ui.tvSnippets->resizeColumnToContents(0);
}

void PagePapers::onEditSnippet(const QModelIndex& idx)
{
	AddSnippetDlg* dlg = new AddSnippetDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onResetPapers()));
	dlg->setWindowTitle(tr("Edit Snippet"));
	dlg->setSnippetID(getSnippetID(idx));
	dlg->show();
}

void PagePapers::onDelSnippets()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tvSnippets->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delSnippet(getSnippetID(idx));
		onResetPapers();
	}
}

int PagePapers::getSnippetID(const QModelIndex& idx) const {
	return ::getSnippetID(modelSnippets.data(modelSnippets.index(idx.row(), 0)).toString());
}

void PagePapers::jumpToPaper(const QString& title) {
	selectID(::getPaperID(title));
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

void PagePapers::saveSectionSizes() {
	ui.tvPapers->saveSectionSizes();
}

void PagePapers::loadSplitterSizes()
{
	ui.splitterHorizontal->restoreState(setting->value("SplitterHorizontal").toByteArray());
	ui.splitterPapers    ->restoreState(setting->value("SplitterPapers")    .toByteArray());
	ui.splitterDetails   ->restoreState(setting->value("SplitterDetails")   .toByteArray());
	ui.splitterTags      ->restoreState(setting->value("SplitterTags")      .toByteArray());
}

void PagePapers::saveSplitterSizes()
{
	setting->setValue("SplitterHorizontal", ui.splitterHorizontal->saveState());
	setting->setValue("SplitterPapers",     ui.splitterPapers->saveState());
	setting->setValue("SplitterDetails",    ui.splitterDetails->saveState());
	setting->setValue("SplitterTags",       ui.splitterTags->saveState());
}