#include "PagePapers.h"
#include "Common.h"
#include "OptionDlg.h"
#include "PaperDlg.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QSqlQuery>
#include <QMenu>

PagePapers::PagePapers(QWidget *parent)
	: QWidget(parent)
{
	currentRowPapers = -1;
	currentRowTags   = -1;

	ui.setupUi(this);
	onShowSearch(false);

	resetPapers();
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);
	resetAllTags();
	modelAllTags.setEditStrategy(QSqlTableModel::OnManualSubmit);

	mapper = new QDataWidgetMapper(this);
	mapper->setModel(&modelPapers);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper->addMapping(ui.teNote,     PAPER_NOTE);

	ui.tableViewPapers->setModel(&modelPapers);
	ui.tableViewPapers->hideColumn(PAPER_ID);
	ui.tableViewPapers->hideColumn(PAPER_JOURNAL);
	ui.tableViewPapers->hideColumn(PAPER_ABSTRACT);
	ui.tableViewPapers->hideColumn(PAPER_NOTE);
	ui.tableViewPapers->hideColumn(PAPER_PROXIMITY);
	ui.tableViewPapers->hideColumn(PAPER_COAUTHOR);
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
	ui.tableViewPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
	ui.tableViewPapers->resizeColumnToContents(PAPER_TITLE);

	ui.listViewAllTags->setModel(&modelAllTags);
	ui.listViewAllTags->setModelColumn(TAG_NAME);

	ui.listViewTags->setModel(&modelTags);

	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowPapersChanged(QModelIndex)));
	connect(ui.tableViewPapers->horizontalHeader(), SIGNAL(sectionPressed(int)),
			this, SLOT(onSubmitPaper()));
	connect(ui.btAddPaper, SIGNAL(clicked()), this, SLOT(onAddPaper()));
	connect(ui.tableViewPapers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditPaper()));
	connect(ui.tableViewPapers, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
	connect(ui.btDelPaper, SIGNAL(clicked()), this, SLOT(onDelPaper()));
	connect(ui.btImport,   SIGNAL(clicked()), this, SLOT(onImport()));
	connect(ui.btSearch,   SIGNAL(toggled(bool)), this, SLOT(onShowSearch(bool)));
	connect(ui.leSearch,   SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
	connect(ui.btCancelSearch, SIGNAL(clicked()), this, SLOT(onCancelSearch()));

	connect(ui.listViewAllTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowAllTagsChanged()));
	connect(ui.btAddTag,  SIGNAL(clicked()), this, SLOT(onAddTag()));
	connect(ui.listViewAllTags, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditTag()));
	connect(ui.btDelTag,  SIGNAL(clicked()), this, SLOT(onDelTag()));
	connect(ui.btAddTagToPaper,   SIGNAL(clicked()), this, SLOT(onAddTagToPaper()));
	connect(ui.btDelTagFromPaper, SIGNAL(clicked()), this, SLOT(onDelTagFromPaper()));
	connect(ui.btFilter, SIGNAL(clicked(bool)), this, SLOT(onFilter(bool)));

	connect(ui.listViewTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowTagsChanged()));

	connect(ui.tableViewPapers, SIGNAL(showRelated()),    this, SLOT(onShowRelated()));
	connect(ui.tableViewPapers, SIGNAL(showCoauthored()), this, SLOT(onShowCoauthored()));
}

void PagePapers::onCurrentRowPapersChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	ui.btDelPaper->setEnabled(valid);
	ui.btAddTag->setEnabled(valid);
	ui.btDelTag->setEnabled(false);

	if(valid)
		onClicked(idx);
}

void PagePapers::onAddPaper()
{
	onSubmitPaper();
	PaperDlg dlg(this);
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
		onSubmitPaper();
	}
}

void PagePapers::onDelPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableViewPapers->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaper(currentPaperID);
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
		ui.tableViewPapers->selectRow(currentRowPapers);
		ui.tableViewPapers->setFocus();
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
	QString lastPath = MySetting<UserSetting>::getInstance()->getLastImportPath();
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Select one or more files to open", lastPath,
		"Reference (*.enw *.ris *.txt);;All files (*.*)");
	if(files.isEmpty())
		return;

	onSubmitPaper();
	ui.tableViewPapers->sortByColumn(PAPER_ID, Qt::AscendingOrder);

	QString file = files.front();
	MySetting<UserSetting>::getInstance()->setLastImportPath(QFileInfo(file).absolutePath());

	foreach(QString fileName, files)
	{
		if(fileName.endsWith(".enw", Qt::CaseInsensitive))
			import(fileName, "%0", 
					QStringList() << "%T", 
					"%A",
					"%D",
					QStringList() << "%J" << "%B", 
					" ");
		else if(fileName.endsWith(".ris", Qt::CaseInsensitive))
			import(fileName, "TY", 
					QStringList() << "TI" << "T1",
					"AU", 
					"PY",
					QStringList() << "JA" << "T2" << "T3", 
					" - ", 
					"AB");
		else if(fileName.endsWith(".txt", Qt::CaseInsensitive))
			import(fileName, "Reference Type", 
					QStringList() << "Title", "Author",
					"UNKOWN_YET!!!",
					QStringList() << "Journal", 
					": ",
					"Abstract");
		else
			import(fileName, "TY", 
					QStringList() << "TI" << "T1",
					"AU", 
					"PY",
					QStringList() << "JA" << "T2" << "T3", 
					" - ", 
					"AB");
	}

	onSubmitPaper();

	int backup = currentPaperID;
	ui.tableViewPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
	currentPaperID = backup;
	selectID(currentPaperID);
}

void PagePapers::import(const QString& fileName,       const QString& firstHead,
						const QStringList& titleHeads, const QString& authorHead, 
						const QString& yearHead,       const QStringList& journalHeads, 
						const QString& delimiter,      const QString& abstractHead)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return;

	QTextStream is(&file);
	while(!is.atEnd())
	{
		QString line = is.readLine();
		int currentRow;
		if(line.startsWith(firstHead))
		{
			currentRow = modelPapers.rowCount();
			modelPapers.insertRow(currentRow);
			currentPaperID = getNextID("Papers", "ID");
			modelPapers.setData(modelPapers.index(currentRow, PAPER_ID), currentPaperID);
			onSubmitPaper();
			continue;
		}

		QString trimmed = trimHead(line, delimiter);
		foreach(QString titleHead, titleHeads)
			if(line.startsWith(titleHead))
			{
				modelPapers.setData(modelPapers.index(currentRow, PAPER_TITLE), trimmed);
				continue;
			}
		
		if(line.startsWith(authorHead))
		{
			QString authors = modelPapers.data(modelPapers.index(currentRow, PAPER_AUTHORS)).toString();
			if(!authors.isEmpty())
				authors.append("; ");
			authors.append(trimmed);
			modelPapers.setData(modelPapers.index(currentRow, PAPER_AUTHORS), authors);
			continue;
		}

		if(line.startsWith(yearHead))
		{
			modelPapers.setData(modelPapers.index(currentRow, PAPER_YEAR), trimmed);
			continue;
		}

		foreach(QString journalHead, journalHeads)
			if(line.startsWith(journalHead))
			{
				QString journals = modelPapers.data(modelPapers.index(currentRow, PAPER_JOURNAL)).toString();
				if(!journals.isEmpty())
					journals.append("; ");
				journals.append(trimmed);
				modelPapers.setData(modelPapers.index(currentRow, PAPER_JOURNAL), journals);
				continue;
			}

		if(line.startsWith(abstractHead))
		{
			modelPapers.setData(modelPapers.index(currentRow, PAPER_ABSTRACT), trimmed);
			continue;
		}
	}
}

QString PagePapers::trimHead(const QString& line, const QString& delimiter) const {
	return line.mid(line.indexOf(delimiter) + delimiter.length());
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
		resetPapers();
	else
		modelPapers.setFilter(
		tr("Title    like \'%%1%\' or \
		    Authors  like \'%%1%\' or \
			Year     like \'%%1%\' or \
			Journal  like \'%%1%\' or \
			Abstract like \'%%1%\' or \
			Note     like \'%%1%\' ").arg(target));
}

void PagePapers::onCancelSearch() {
	ui.btSearch->setChecked(false);
}

void PagePapers::onCurrentRowAllTagsChanged()
{
	QModelIndexList idxList = ui.listViewAllTags->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRowTags = valid ? idxList.front().row() : -1;	
	ui.btDelTag ->setEnabled(valid);
	ui.btAddTagToPaper->setEnabled(valid && !isFiltered());

	if(isFiltered())
	{
		if(currentRowTags < 0)
		{
			resetPapers();
			return;
		}
		filterPapers();
	}
}

void PagePapers::onAddTag()
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

void PagePapers::onDelTag()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.listViewAllTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delTag(getAllTagID(idx.row()));
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
	QModelIndexList idxList = ui.listViewAllTags->selectionModel()->selectedRows();
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
	ui.listViewTags->setModelColumn(TAG_NAME);

	if(!isFiltered())
		modelAllTags.setFilter(tr("ID not in %1 order by Name").arg(thisPapersTags));
}

void PagePapers::onCurrentRowTagsChanged() {
	ui.btDelTagFromPaper->setEnabled(!ui.listViewTags->selectionModel()->selectedRows().isEmpty());
}

void PagePapers::onDelTagFromPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.listViewTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaperTag(currentPaperID, getTagID(idx.row()));
		updateTags();
	}
}

void PagePapers::filterPapers()
{
	hideRelated();
	hideCoauthor();
	QStringList tagClauses;
	QModelIndexList idxList = ui.listViewAllTags->selectionModel()->selectedRows();
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
	{
		resetPapers();    // show all papers
		selectID(currentPaperID);
	}
}

void PagePapers::resetPapers()
{
	modelPapers.setTable("Papers");
	modelPapers.select();
	ui.tableViewPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
	hideRelated();
	hideCoauthor();
}

void PagePapers::resetAllTags()
{
	modelAllTags.setTable("Tags");
	modelAllTags.setFilter("Name != \'\' order by Name");
	modelAllTags.select();
}

void PagePapers::onShowSearch(bool enable)
{
	if(enable)
	{
		ui.frameSearch->show();
		ui.leSearch->setFocus();
	}
	else
	{
		ui.leSearch->clear();
		ui.frameSearch->hide();
		resetPapers();
	}
}

bool PagePapers::isFiltered() const {
	return ui.btFilter->isChecked();
}

void PagePapers::resizeEvent(QResizeEvent*)
{
	ui.splitterHorizontal->setSizes(QList<int>() << width()  * 0.85 << width()  * 0.15);
	ui.splitterPapers    ->setSizes(QList<int>() << height() * 0.6 << height() * 0.4);
	ui.splitterTags      ->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);
	ui.splitterDetails->setSizes(QList<int>() << width() * 0.45 << width() * 0.45 << width() * 0.1);
}

void PagePapers::onClicked(const QModelIndex& idx)
{
	currentRowPapers = idx.row();
	currentPaperID = getPaperID(currentRowPapers);
	updateTags();
	ui.widgetAttachments->setPaper(currentPaperID);
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

	ui.tableViewPapers->sortByColumn(PAPER_PROXIMITY, Qt::DescendingOrder);
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
	QSqlDatabase::database().transaction();
	QSqlQuery query;	
	query.exec(tr("update Papers set Coauthor = 0"));
	QStringList authors = modelPapers.data(modelPapers.index(currentRowPapers, PAPER_AUTHORS)).toString().split(";");
	foreach(QString author, authors)
		query.exec(tr("update Papers set Coauthor = Coauthor + 1 where Authors like \'%%1%\'")
															.arg(author.trimmed()));
	query.exec(tr("update Papers set Coauthor = (select max(Coauthor)+1 from Papers) \
													where ID = %1").arg(currentPaperID));
	QSqlDatabase::database().commit();

	ui.tableViewPapers->sortByColumn(PAPER_COAUTHOR, Qt::DescendingOrder);
	selectID(currentPaperID);
}

void PagePapers::hideCoauthor()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.exec(tr("update Papers set Coauthor = 0"));
	QSqlDatabase::database().commit();
}
