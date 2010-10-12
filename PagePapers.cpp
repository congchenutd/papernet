#include "PagePapers.h"
#include "Common.h"
#include "OptionDlg.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>

PagePapers::PagePapers(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.splitterHorizontal->setSizes(QList<int>() << width()  * 0.9 << width()  * 0.1);
	ui.splitterPapers    ->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);
	ui.splitterTags      ->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);

	modelPapers.setTable("Papers");
	modelPapers.setEditStrategy(QSqlTableModel::OnManualSubmit);
	modelPapers.select();
	modelAllTags.setTable("Tags");
	modelAllTags.setFilter("Name != \'\' order by Name");
	modelAllTags.setEditStrategy(QSqlTableModel::OnManualSubmit);
	modelAllTags.select();

	mapper = new QDataWidgetMapper(this);
	mapper->setModel(&modelPapers);
	mapper->addMapping(ui.leTitle,    PAPER_TITLE);
	mapper->addMapping(ui.leAuthors,  PAPER_AUTHORS);
	mapper->addMapping(ui.leJournal,  PAPER_JOURNAL);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);
	mapper->addMapping(ui.teNote,     PAPER_NOTE);

	ui.tableViewPapers->setModel(&modelPapers);
	ui.tableViewPapers->hideColumn(PAPER_ID);
	ui.tableViewPapers->hideColumn(PAPER_ABSTRACT);
	ui.tableViewPapers->hideColumn(PAPER_NOTE);
	ui.tableViewPapers->hideColumn(PAPER_PDF);
	ui.tableViewPapers->resizeColumnsToContents();
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
	ui.tableViewPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);

	ui.listViewAllTags->setModel(&modelAllTags);
	ui.listViewAllTags->setModelColumn(TAG_NAME);

	ui.listViewTags->setModel(&modelTags);

	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowPapersChanged(QModelIndex)));
	connect(ui.btAddPaper, SIGNAL(clicked()), this, SLOT(onAddPaper()));
	connect(ui.btDelPaper, SIGNAL(clicked()), this, SLOT(onDelPaper()));
	connect(ui.btImport,   SIGNAL(clicked()), this, SLOT(onImport()));
	connect(ui.leSearch,   SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
	connect(ui.btCancelSearch, SIGNAL(clicked()), this, SLOT(onCancelSearch()));
	connect(ui.btSetPDF,  SIGNAL(clicked()), this, SLOT(onSetPDF()));
	connect(ui.btReadPDF, SIGNAL(clicked()), this, SLOT(onReadPDF()));

	connect(ui.listViewAllTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowAllTagsChanged()));
	connect(ui.btAddTag,  SIGNAL(clicked()), this, SLOT(onAddTag()));
	connect(ui.listViewAllTags, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditTag()));
	connect(ui.btDelTag,  SIGNAL(clicked()), this, SLOT(onDelTag()));
	connect(ui.btAddTagToPaper,   SIGNAL(clicked()), this, SLOT(onAddTagToPaper()));
	connect(ui.btDelTagFromPaper, SIGNAL(clicked()), this, SLOT(onDelTagFromPaper()));

	connect(ui.listViewTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowTagsChanged()));
}

void PagePapers::onCurrentRowPapersChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	currentRowPapers = valid ? idx.row() : -1;
	ui.btDelPaper->setEnabled(valid);
	ui.btAddTag->setEnabled(valid);
	ui.btDelTag->setEnabled(false);
	ui.btSetPDF->setEnabled(valid);

	if(valid)
	{
		updateTags();
//		updateRelatedPapers();
		ui.btReadPDF->setEnabled(!getCurrentPDFPath().isEmpty());
	}
}

void PagePapers::onAddPaper()
{
	submitPaper();
	int lastRow = modelPapers.rowCount();
	modelPapers.insertRow(lastRow);
	int nextID = getNextID("Papers", "ID");
	modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), nextID);
	submitPaper();
	selectID(nextID);
	ui.leTitle->setFocus();
}

void PagePapers::onDelPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		bool keepPDF = MySetting<UserSetting>::getInstance()->getKeepPDF();
		int paperID = getCurrentPaperID();
		if(!keepPDF)
			delPDF(paperID);
		::delPaper(paperID);
		modelPapers.select();
	}
}

int PagePapers::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}
int PagePapers::getTagID(int row) const {
	return row > -1 ? modelAllTags.data(modelAllTags.index(row, TAG_ID)).toInt() : -1;
}
int PagePapers::getCurrentPaperID() const {
	return getPaperID(currentRowPapers);
}

void PagePapers::selectID(int id)
{
	int row = idToRow(id);
	if(row > -1)
	{
		currentRowPapers = row;
		ui.tableViewPapers->selectRow(currentRowPapers);
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
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Select one or more files to open", ".",
		"Reference (*.enw *.ris *.txt);;All files (*.*)");
	if(files.isEmpty())
		return;
	foreach(QString fileName, files)
	{
		if(fileName.endsWith(".enw", Qt::CaseInsensitive))
			import(fileName, "%0", "%T", "%A", QStringList() << "%J" << "%B", " ");
		else if(fileName.endsWith(".ris", Qt::CaseInsensitive))
			import(fileName, "TY", "TI", "AU", QStringList() << "JA", " - ", "AB");
		else if(fileName.endsWith(".txt", Qt::CaseInsensitive))
			import(fileName, "Reference Type", "Title", "Author", QStringList() << "Journal", ": ", "Abstract");
		else
			import(fileName, "TY", "TI", "AU", QStringList() << "JA", " - ", "AB");
	}
}

void PagePapers::import(const QString& fileName,    const QString& firstHead,
						const QString& titleHead,   const QString& authorHead, 
						const QStringList& journalHeads, const QString& delimiter,
						const QString& abstractHead)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return;

	ui.tableViewPapers->sortByColumn(PAPER_ID, Qt::AscendingOrder);
	QTextStream is(&file);
	while(!is.atEnd())
	{
		QString line = is.readLine();
		int currentRow;
		if(line.startsWith(firstHead))
		{
			currentRow = modelPapers.rowCount();
			modelPapers.insertRow(currentRow);
			int nextID = getNextID("Papers", "ID");
			modelPapers.setData(modelPapers.index(currentRow, PAPER_ID), nextID);
			submitPaper();
			continue;
		}

		QString trimmed = trimHead(line, delimiter);
		if(line.startsWith(titleHead))
			modelPapers.setData(modelPapers.index(currentRow, PAPER_TITLE), trimmed);
		else if(line.startsWith(authorHead))
		{
			QString authors = modelPapers.data(modelPapers.index(currentRow, PAPER_AUTHORS)).toString();
			if(!authors.isEmpty())
				authors.append(", ");
			authors.append(trimmed);
			modelPapers.setData(modelPapers.index(currentRow, PAPER_AUTHORS), authors);
		}
		else if(line.startsWith(abstractHead))
			modelPapers.setData(modelPapers.index(currentRow, PAPER_ABSTRACT), trimmed);
		else 
		{
			foreach(QString journalHead, journalHeads)
				if(line.startsWith(journalHead))
					modelPapers.setData(modelPapers.index(currentRow, PAPER_JOURNAL), trimmed);
		}
	}
	submitPaper();
	ui.tableViewPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
}

QString PagePapers::trimHead(const QString& line, const QString& delimiter) const {
	return line.mid(line.indexOf(delimiter) + delimiter.length());
}

void PagePapers::submitPaper() {
	modelPapers.submitAll();
}

PagePapers::~PagePapers() {
	submitPaper();
}

void PagePapers::onSearch(const QString& target)
{
	if(target.isEmpty())
		onCancelSearch();
	else
		modelPapers.setFilter(
		tr("Title like \'%%1%\' or Authors like \'%%1%\' or Journal like \'%%1%\'").arg(target));
}

void PagePapers::onCancelSearch()
{
	ui.leSearch->clear();
	modelPapers.setTable("Papers");
	modelPapers.select();
}

QString PagePapers::getCurrentPDFPath() const {
	return modelPapers.data(
				modelPapers.index(currentRowPapers, PAPER_PDF)).toString();
}

void PagePapers::onSetPDF()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		".", tr("PDF file (*.pdf)"));
	if(fileName.isEmpty())
		return;
	QDir(".").mkdir("PDF");
	QString title = modelPapers.data(modelPapers.index(currentRowPapers, PAPER_TITLE)).toString();
	QString path = ".\\PDF\\" + makePDFFileName(title) + ".pdf";
	QFile::copy(fileName, path);
	modelPapers.setData(modelPapers.index(currentRowPapers, PAPER_PDF), path);
}

QString PagePapers::makePDFFileName(const QString& title) const {
	return QString(title).replace(QRegExp("[:|?|*]"), "-");
}

void PagePapers::onReadPDF()
{
	QString path = getCurrentPDFPath();
	if(QFile::exists(path))
	{
		QDesktopServices::openUrl(QUrl(path));
		return;
	}

	if(QMessageBox::warning(this, "Warning", "The PDF file path is not valid, do you want to find the file?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		onSetPDF();
		return;
	}

	// delete pdf record
//	modelPapers.setData(modelPapers.index(currentRowPaper, PAPER_PDF), QString());
}

void PagePapers::onCurrentRowAllTagsChanged()
{
	QModelIndexList idxList = ui.listViewAllTags->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRowTags = valid ? idxList.front().row() : -1;	
	ui.btDelTag ->setEnabled(valid);
	ui.btAddPaper->setEnabled(valid);
	ui.btDelPaper->setEnabled(valid);
	ui.btAddTagToPaper->setEnabled(valid);

	if(valid)
	{
		updatePapers();
//		updateRelatedTags();
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
			::delTag(getTagID(idx.row()));
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
	int paperID = getCurrentPaperID();
	QModelIndexList idxList = ui.listViewAllTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		addPaperTag(paperID, getTagID(idx.row()));
	updateTags();
}

void PagePapers::updateTags()
{
	int paper = getCurrentPaperID();
	modelTags.setQuery(
		tr("select Name from Tags where ID in \
		   (select Tag from PaperTag where Paper = %1) order by Name").arg(paper));
}

void PagePapers::onCurrentRowTagsChanged() {
	ui.btDelTagFromPaper->setEnabled(!ui.listViewTags->selectionModel()->selectedRows().isEmpty());
}

void PagePapers::onDelTagFromPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		int id = getCurrentPaperID();
		QModelIndexList idxList = ui.listViewTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaperTag(id, getTagID(idx.row()));
		updateTags();
	}
}

void PagePapers::updatePapers()
{
	QStringList tagClauses;
	QModelIndexList idxList = ui.listViewTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		tagClauses << tr("Tag = %1").arg(getTagID(idx.row()));
	if(tagClauses.isEmpty())
		tagClauses << tr("Tag = %1").arg(getTagID(currentRowTags));
	modelPapers.setFilter(tr("ID in (select Paper from PaperTag where %1)")
									.arg(tagClauses.join(" OR ")));
	QString test = tr("ID in (select Paper from PaperTag where %1)")
		.arg(tagClauses.join(" OR "));
}
