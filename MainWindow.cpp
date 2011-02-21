#include "MainWindow.h"
#include "OptionDlg.h"
#include "Common.h"
#include "Common.h"
#include "OptionDlg.h"
#include "PaperDlg.h"
#include "AddSnippetDlg.h"
#include "Importer.h"
#include "../EnglishName/EnglishName.h"
#include <QMessageBox>
#include <QDate>
#include <QActionGroup>
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QSqlQuery>
#include <QMenu>
#include <QFileInfo>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	connect(ui.actionOptions,     SIGNAL(triggered()), this, SLOT(onOptions()));
	connect(ui.actionAbout,       SIGNAL(triggered()), this, SLOT(onAbout()));

	// load settings
	qApp->setFont(MySetting<UserSetting>::getInstance()->getFont());


	currentRowPapers = -1;
	currentRowTags   = -1;

	onResetPapers();
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);
	resetAllTags();
	modelAllTags.setEditStrategy(QSqlTableModel::OnManualSubmit);

	mapper = new QDataWidgetMapper(this);
	mapper->setModel(&modelPapers);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper->addMapping(ui.teNote,     PAPER_NOTE);

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

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
		mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
		this, SLOT(onCurrentRowPapersChanged(QModelIndex)));
	connect(ui.tvPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
		this, SLOT(onSubmitPaper()));
	connect(ui.actionAddPaper, SIGNAL(triggered()), this, SLOT(onAddPaper()));
	connect(ui.actionDelPaper, SIGNAL(triggered()), this, SLOT(onDelPaper()));
	connect(ui.actionImport,   SIGNAL(triggered()), this, SLOT(onImport()));
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
	connect(ui.tvPapers, SIGNAL(addSnippet()),     this, SLOT(onAddSnippet()));
}

void MainWindow::onOptions()
{
	OptionDlg dlg(this);
	dlg.exec();
}

void MainWindow::onAbout()
{
	QMessageBox::about(this, "About", 
		tr("<h3><b>PaperNet: a better paper manager</b></h3>"
		"<p>Build 2011.2.19</p>"
		"<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>"));
}

void MainWindow::closeEvent(QCloseEvent*)
{
	UserSetting* setting = MySetting<UserSetting>::getInstance();
	if(setting->getBackupDays() > 0)
	{
		delOldBackup();
		backup();
	}
	setting->destroySettingManager();
}

void MainWindow::delOldBackup()
{
	const QDate today = QDate::currentDate();
	const int   days  = MySetting<UserSetting>::getInstance()->getBackupDays();
	const QFileInfoList fileInfos = 
		QDir("Backup").entryInfoList(QStringList() << "*.db", QDir::Files);
	foreach(QFileInfo fileInfo, fileInfos)
		if(QDate::fromString(fileInfo.baseName(), Qt::ISODate).daysTo(today) > days)
			QFile::remove(fileInfo.filePath());
}

void MainWindow::backup(const QString& name)
{
	QDir::current().mkdir("Backup");
	QString backupFileName = name.isEmpty() 
		? "./Backup/" + QDate::currentDate().toString(Qt::ISODate) + ".db" 
		: name;

	if(QFile::exists(backupFileName))
		QFile::remove(backupFileName);

	extern QString dbName;
	QFile file(dbName);
	file.copy(backupFileName);
	file.close();
}

void MainWindow::onCurrentRowPapersChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	ui.actionDelPaper->setEnabled(valid);
	ui.btDelTag->setEnabled(false);

	if(valid)
		onClicked(idx);
}

void MainWindow::onAddPaper()
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

void MainWindow::onEditPaper()
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
			setRead(currentPaperID);
		onSubmitPaper();
	}
}

void MainWindow::onDelPaper()
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

int MainWindow::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}
int MainWindow::getTagID(int row) const {
	return row > -1 ? modelTags.data(modelTags.index(row, TAG_ID)).toInt() : -1;
}
int MainWindow::getAllTagID(int row) const {
	return row > -1 ? modelAllTags.data(modelAllTags.index(row, TAG_ID)).toInt() : -1;
}

void MainWindow::selectID(int id)
{
	int row = idToRow(id);
	if(row > -1)
	{
		currentRowPapers = row;
		ui.tvPapers->selectRow(currentRowPapers);
		ui.tvPapers->setFocus();
	}
}

int MainWindow::idToRow(int id) const
{
	QModelIndexList indexes = modelPapers.match(
		modelPapers.index(0, PAPER_ID), Qt::DisplayRole, id, 1, Qt::MatchExactly | Qt::MatchWrap);
	return !indexes.isEmpty() ? indexes.at(0).row() : -1;
}

void MainWindow::onImport()
{
	QString lastPath = MySetting<UserSetting>::getInstance()->getLastImportPath();
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Select one or more files to open", lastPath,
		"Reference (*.enw *.ris *.xml);;All files (*.*)");
	if(files.isEmpty())
		return;

	onSubmitPaper();
	ui.tvPapers->sortByColumn(PAPER_ID, Qt::AscendingOrder);

	QString file = files.front();
	MySetting<UserSetting>::getInstance()->setLastImportPath(QFileInfo(file).absolutePath());

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

void MainWindow::insertRecord(const ImportResult &record)
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

void MainWindow::mergeRecord(int row, const ImportResult &record)
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

void MainWindow::onSubmitPaper() 
{
	hideRelated();
	hideCoauthor();
	int backup = currentPaperID;
	modelPapers.submitAll();
	currentPaperID = backup;
	selectID(backup);
}

MainWindow::~MainWindow() {
	onSubmitPaper();
}

void MainWindow::onSearch(const QString& target)
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

void MainWindow::onCurrentRowAllTagsChanged()
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

void MainWindow::onAddTag()
{
	bool ok;
	QString tag = QInputDialog::getText(this, "Add Tag", "Tag Name", 
		QLineEdit::Normal,	"New Tag", &ok);
	if(!ok || tag.isEmpty())
		return;
	int lastRow = modelAllTags.rowCount();
	modelAllTags.insertRow(lastRow);
	modelAllTags.setData(modelAllTags.index(lastRow, TAG_ID), getNextID("Tags", "ID"));
	modelAllTags.setData(modelAllTags.index(lastRow, TAG_NAME), tag);
	modelAllTags.submitAll();
}

void MainWindow::onDelTag()
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

void MainWindow::onEditTag()
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

void MainWindow::onAddTagToPaper()
{
	QModelIndexList idxList = ui.lvAllTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		addPaperTag(currentPaperID, getAllTagID(idx.row()));
	updateTags();
}

void MainWindow::updateTags()
{
	QString thisPapersTags(tr("(select Tag from PaperTag where Paper = %1)")
		.arg(currentPaperID));
	modelTags.setQuery(tr("select ID, Name from Tags where ID in %1 order by Name")
		.arg(thisPapersTags));
	ui.lvTags->setModelColumn(TAG_NAME);

	if(!isFiltered())
		modelAllTags.setFilter(tr("ID not in %1 order by Name").arg(thisPapersTags));
}

void MainWindow::onCurrentRowTagsChanged() {
	ui.btDelTagFromPaper->setEnabled(!ui.lvTags->selectionModel()->selectedRows().isEmpty());
}

void MainWindow::onDelTagFromPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.lvTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaperTag(currentPaperID, getTagID(idx.row()));
		updateTags();
	}
}

void MainWindow::filterPapers()
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

void MainWindow::onFilter(bool enabled)
{
	if(enabled)
		resetAllTags();   // show all tags
	else
		onResetPapers();    // show all papers
}

void MainWindow::onResetPapers()
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

void MainWindow::resetAllTags()
{
	modelAllTags.setTable("Tags");
	modelAllTags.setFilter("Name != \"\" order by Name");
	modelAllTags.select();
}

bool MainWindow::isFiltered() const {
	return ui.btFilter->isChecked();
}

void MainWindow::resizeEvent(QResizeEvent*)
{
	//ui.splitterHorizontal->setSizes(QList<int>() << width()  * 0.85 << width()  * 0.15);
	//ui.splitterPapers    ->setSizes(QList<int>() << height() * 0.6 << height() * 0.4);
	//ui.splitterTags      ->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);
	//   ui.splitterDetails   ->setSizes(QList<int>() << width() * 0.5 << width() * 0.35 << width() * 0.15);
}

void MainWindow::onClicked(const QModelIndex& idx)
{
	currentRowPapers = idx.row();
	currentPaperID = getPaperID(currentRowPapers);
	updateTags();
	ui.widgetAttachments->setPaper(currentPaperID);
}

void MainWindow::onShowRelated()
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

void MainWindow::hideRelated()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Proximity = 0"));
	QSqlDatabase::database().commit();
}

void MainWindow::onShowCoauthored()
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

void MainWindow::hideCoauthor()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Coauthor = 0"));
	QSqlDatabase::database().commit();
}

void MainWindow::onAddSnippet()
{
	AddSnippetDlg dlg(this);
	dlg.setWindowTitle(tr("Add Snippet"));
	dlg.setSnippetID(getNextID("Snippets", "ID"));
	dlg.addPaper(getPaperTitle(currentPaperID));
	if(dlg.exec() == QDialog::Accepted)
		onResetPapers();
}