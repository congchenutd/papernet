#include "PaperTagPage.h"
#include "Common.h"
#include <QDataWidgetMapper>
#include <QStringList>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

PaperTagPage::PaperTagPage(QWidget *parent)
	: QWidget(parent), mode(0), currentRowPaper(-1)
{
	ui.setupUi(this);

	mapper = new QDataWidgetMapper(this);
	mapper->setModel(&modelPapers);
	mapper->addMapping(ui.leTitle,    PAPER_TITLE);
	mapper->addMapping(ui.leAuthors,  PAPER_AUTHORS);
	mapper->addMapping(ui.leJournal,  PAPER_JOURNAL);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);

	ui.tableViewPapers->setModel(&modelPapers);
	ui.listViewTags->setModel(&modelTags);
	ui.listViewRelatedPapers->setModel(&modelRelatedPapers);
	ui.listViewRelatedTags  ->setModel(&modelRelatedTags);

	ui.splitterHorizontal->setSizes(QList<int>() << width() * 0.9 << width() * 0.1);
	ui.splitterPapers->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);

	connect(ui.btByWhat, SIGNAL(clicked()), this, SLOT(onByWhat()));
	connect(ui.btAddPaper, SIGNAL(clicked()), this, SLOT(onAddPaper()));
	connect(ui.btDelPaper, SIGNAL(clicked()), this, SLOT(onDelPaper()));
	connect(ui.btSave,   SIGNAL(clicked()), this,   SLOT(onSubmitPaper()));
	connect(ui.btCancel, SIGNAL(clicked()), &modelPapers, SLOT(revert()));
	connect(ui.btImport,  SIGNAL(clicked()), this, SLOT(onImport()));
	connect(ui.btSetPDF,  SIGNAL(clicked()), this, SLOT(onSetPDF()));
	connect(ui.btReadPDF, SIGNAL(clicked()), this, SLOT(onReadPDF()));
	connect(ui.btCancelSearch, SIGNAL(clicked()), this, SLOT(onCancelSearch()));
	connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
	connect(ui.btAddTag,  SIGNAL(clicked()), this, SLOT(onAddTag()));
	connect(ui.btEditTag, SIGNAL(clicked()), this, SLOT(onEditTag()));
	connect(ui.btDelTag,  SIGNAL(clicked()), this, SLOT(onDelTag()));

	mode = 0;
//	onByWhat();
//	changeMode();
//	mode = new TagMode(this);
}

void PaperTagPage::changeMode()
{
	if(mode == 0)
	{
		mode = new PaperMode(this);
		mode->enter();
		return;
	}

	mode->leave();
	Mode* lastMode = mode;
	if(lastMode->getName() == "TagMode")
		mode = new PaperMode(this);
	else
		mode = new TagMode(this);
	
	delete lastMode;
	mode->enter();
}

void PaperTagPage::onByWhat()
{
//	ui.btByTag->setChecked(!ui.btByPaper->isChecked());
	changeMode();
}

void PaperTagPage::resetViews()
{
	ui.tableViewPapers->hideColumn(PAPER_ID);
	ui.tableViewPapers->hideColumn(PAPER_ABSTRACT);
	ui.tableViewPapers->hideColumn(PAPER_PDF);
	ui.tableViewPapers->resizeColumnsToContents();
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
	ui.listViewTags->setModelColumn(TAG_NAME);

	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowPaperChanged(QModelIndex)));
	connect(ui.listViewTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowTagChanged()));
}

PaperTagPage::~PaperTagPage()
{
	if(mode != 0)
		delete mode;
}

void PaperTagPage::onCurrentRowPaperChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	currentRowPaper = valid ? idx.row() : -1;
	ui.btDelPaper->setEnabled(valid);
	ui.btAddTag->setEnabled(valid);
	ui.btDelTag->setEnabled(false);
	ui.btSave->setEnabled(valid);
	ui.btCancel->setEnabled(valid);
	ui.btSetPDF->setEnabled(valid);

	if(valid)
	{
		mode->updateTags();
		updateRelatedPapers();
		ui.btReadPDF->setEnabled(!getPDFPath().isEmpty());
	}
}

void PaperTagPage::onImport()
{
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Select one or more files to open", ".",
		"Reference (*.enw *.ris *.txt);;All files (*.*)");
	if(files.isEmpty())
		return;
	foreach(QString fileName, files)
	{
		if(fileName.endsWith(".enw", Qt::CaseInsensitive))
			import(fileName, "%0", "%T", "%A", "%J", " ");
		else if(fileName.endsWith(".ris", Qt::CaseInsensitive))
			import(fileName, "TY", "TI", "AU", "JA", " - ", "AB");
		else if(fileName.endsWith(".txt", Qt::CaseInsensitive))
			import(fileName, "Reference Type", "Title", "Author", "Journal", ": ", "Abstract");
	}
}

void PaperTagPage::import(const QString& fileName,    const QString& firstHead,
						  const QString& titleHead,   const QString& authorHead, 
						  const QString& journalHead, const QString& delimiter,
						  const QString& abstractHead)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return;

	QTextStream is(&file);
	while(!is.atEnd())
	{
		QString line = is.readLine();
        int currentRow = -1;
		if(line.startsWith(firstHead))
		{
			currentRow = modelPapers.rowCount();
			modelPapers.insertRow(currentRow);
			modelPapers.setData(modelPapers.index(currentRow, PAPER_ID), getNextID("Papers", "ID"));
			onSubmitPaper();
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
		else if(line.startsWith(journalHead))
			modelPapers.setData(modelPapers.index(currentRow, PAPER_JOURNAL), trimmed);
		else if(line.startsWith(abstractHead))
			modelPapers.setData(modelPapers.index(currentRow, PAPER_ABSTRACT), trimmed);
	}
	onSubmitPaper();
}

QString PaperTagPage::trimHead(const QString& line, const QString& delimiter) const {
	return line.mid(line.indexOf(delimiter) + delimiter.length());
}

void PaperTagPage::onAddPaper() {
	mode->addPaper();
}

void PaperTagPage::onDelPaper() {
	mode->delPaper();
}

void PaperTagPage::onSubmitPaper() {
	modelPapers.submit();
}

void PaperTagPage::onSetPDF()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		".", tr("PDF file (*.pdf)"));
	if(fileName.isEmpty())
		return;
	QDir(".").mkdir("PDF");
	QString title = modelPapers.data(modelPapers.index(currentRowPaper, PAPER_TITLE)).toString();
	QString pdfFilePath = ".\\PDF\\" + makePDFFileName(title) + ".pdf";
	QFile::copy(fileName, pdfFilePath);
	modelPapers.setData(modelPapers.index(currentRowPaper, PAPER_PDF), pdfFilePath);
}

QString PaperTagPage::makePDFFileName(const QString& title)
{
	QString result = title;
	result.replace(QRegExp("[:|?|*]"), "-");
	return result;
}

void PaperTagPage::onReadPDF()
{
	QString path = getPDFPath();
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
	modelPapers.setData(modelPapers.index(currentRowPaper, PAPER_PDF), QString());
}

void PaperTagPage::onSearch(const QString& target)
{
	if(target.isEmpty())
		onCancelSearch();
	else
		modelPapers.setFilter(
			tr("Title like \'%%1%\' or Authors like \'%%1%\' or Journal like \'%%1%\'").arg(target));
}

void PaperTagPage::onCancelSearch()
{
	ui.leSearch->clear();
	modelPapers.setTable("Papers");
	modelPapers.select();
}

int PaperTagPage::getCurrentPaperID() const {
	return getPaperID(currentRowPaper);
}
int PaperTagPage::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}

void PaperTagPage::updateRelatedPapers()
{
	int paperID = getCurrentPaperID();
	modelRelatedPapers.setQuery(tr("select Title, count(Paper) Proximity from Papers, PaperTag \
								   where Tag in (select Tag from PaperTag where Paper = %1) \
								   and Paper != %1 and ID = Paper \
								   group by Paper order by Proximity desc").arg(paperID));
}

//////////////////////////////////////////////////////////////////////////
// Tags
void PaperTagPage::onAddTag() {
	mode->addTag();
}

void PaperTagPage::onEditTag() {
	mode->editTag();
}

void PaperTagPage::onDelTag() {
	mode->delTag();
}

QString PaperTagPage::getPDFPath() const {
	return modelPapers.data(modelPapers.index(currentRowPaper, PAPER_PDF)).toString();
}

void PaperTagPage::onCurrentRowTagChanged()
{
	QModelIndexList idxList = ui.listViewTags->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRowTag = valid ? idxList.front().row() : -1;	
	ui.btEditTag->setEnabled(valid);
	ui.btDelTag ->setEnabled(valid);
	ui.btAddPaper->setEnabled(valid);
	ui.btDelPaper->setEnabled(valid);

	if(valid)
	{
		mode->updatePapers();
		updateRelatedTags();
	}
}

int PaperTagPage::getCurrentTagID() const {
	return getTagID(currentRowTag);
}
int PaperTagPage::getTagID(int row) const {
	return row > -1 ? modelTags.data(modelTags.index(row, TAG_ID)).toInt() : -1;
}

void PaperTagPage::updateRelatedTags()
{
	int tagID = getCurrentTagID();
	modelRelatedTags.setQuery(tr("select Name, count(Tag) Proximity from Tags, PaperTag \
								 where Paper in (select Paper from PaperTag where Tag = %1) \
								 and Tag != %1 and ID = Tag \
								 group by Tag order by Proximity desc").arg(tagID));
}

QList<int> PaperTagPage::getSelectedTagIDs() const
{
	QList<int> result;
	QModelIndexList idxList = ui.listViewTags->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		result << getTagID(idx.row());
	if(result.isEmpty())
		result << getCurrentTagID();
	return result;
}
