#include "PaperList.h"
#include "Common.h"
#include "AddPaperTagDlg.h"
#include <QtGui>
#include <map>

using namespace std;

PaperList::PaperList(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	modelPapers.setTable("Papers");
	modelPapers.select();
	modelPapers.setEditStrategy(QSqlTableModel::OnFieldChange);

	ui.tableViewPapers->setModel(&modelPapers);
	ui.tableViewPapers->hideColumn(PAPER_ID);
	ui.tableViewPapers->hideColumn(PAPER_ABSTRACT);
	ui.tableViewPapers->hideColumn(PAPER_PDF);
	ui.tableViewPapers->resizeColumnsToContents();
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);

	QDataWidgetMapper* mapper = new QDataWidgetMapper(this);
	mapper->setModel(&modelPapers);
	mapper->addMapping(ui.leTitle,    PAPER_TITLE);
	mapper->addMapping(ui.leAuthors,  PAPER_AUTHORS);
	mapper->addMapping(ui.leJournal,  PAPER_JOURNAL);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);

	ui.splitterVertical->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);
//	ui.splitterHorizontal->setSizes(QList<int>() << width() * 0.4 << width() * 0.6);

	ui.listViewRelated->setModel(&modelRelatedPapers);
	ui.listViewTags->setModel(&modelTags);

	connect(ui.btAddPaper, SIGNAL(clicked()), this, SLOT(onAddPaper()));
	connect(ui.btDelPaper, SIGNAL(clicked()), this, SLOT(onDelPaper()));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowChanged(QModelIndex)));
	connect(ui.btSave,   SIGNAL(clicked()), this,   SLOT(onSubmitPaper()));
	connect(ui.btCancel, SIGNAL(clicked()), &modelPapers, SLOT(revert()));
	connect(ui.btAddTag,  SIGNAL(clicked()), this, SLOT(onAddTag()));
	connect(ui.btDelTag,  SIGNAL(clicked()), this, SLOT(onDelTag()));
	connect(ui.btImport,  SIGNAL(clicked()), this, SLOT(onImport()));
	connect(ui.btSetPDF,  SIGNAL(clicked()), this, SLOT(onSetPDF()));
	connect(ui.btReadPDF, SIGNAL(clicked()), this, SLOT(onReadPDF()));
	connect(ui.btCancelSearch, SIGNAL(clicked()), this, SLOT(onCancelSearch()));
	connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
	connect(ui.listViewTags->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onEnableDelTags(QModelIndex)));

	onCancelSearch();
}

void PaperList::onAddPaper()
{
	int lastRow = modelPapers.rowCount();
	modelPapers.insertRow(lastRow);
	modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), getNextID("Papers", "ID"));
	ui.tableViewPapers->selectRow(lastRow);
	ui.leTitle->setFocus();
}

void PaperList::onDelPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", QMessageBox::Yes | QMessageBox::No) 
		== QMessageBox::Yes)
	{
		delPaperTagByPaper(getCurrentPaperID());
		modelPapers.removeRow(currentRow);
	}
}

void PaperList::onSubmitPaper()
{
	modelPapers.submit();
	ui.tableViewPapers->resizeColumnsToContents();
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
}

void PaperList::onCurrentRowChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	currentRow = valid ? idx.row() : -1;	
	ui.btDelPaper->setEnabled(valid);
	ui.btAddTag->setEnabled(valid);
	ui.btDelTag->setEnabled(false);
	ui.btSave->setEnabled(valid);
	ui.btCancel->setEnabled(valid);
	ui.btSetPDF->setEnabled(valid);

	if(valid)
	{
		updateTags();
		updateRelatedPapers();
		QString pdfPath = modelPapers.data(modelPapers.index(currentRow, PAPER_PDF)).toString();
		ui.btReadPDF->setEnabled(!pdfPath.isEmpty());
	}
}

int PaperList::getCurrentPaperID() const {
	return currentRow > -1 ? modelPapers.data(modelPapers.index(currentRow, PAPER_ID)).toInt() : -1;
}

void PaperList::updateRelatedPapers()
{
	int paperID = getCurrentPaperID();
	modelRelatedPapers.setQuery(tr("select Title, count(Paper) Proximity from Papers, PaperTag \
								   where Tag in (select Tag from PaperTag where Paper = %1) \
								   and Paper != %1 and ID = Paper \
								   group by Paper order by Proximity desc").arg(paperID));
}

void PaperList::onAddTag()
{
	AddPaperTagDlg dlg(this, "Tags");
	if(dlg.exec() == QDialog::Accepted)
	{
		QList<int> tags = dlg.getSelected();
		int paperID = getCurrentPaperID();
		foreach(int tagID, tags)
			addPaperTag(paperID, tagID);
		updateTags();
	}
}

void PaperList::onDelTag()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", QMessageBox::Yes | QMessageBox::No) 
		== QMessageBox::Yes)
	{
		int id = getCurrentPaperID();
		QModelIndexList idxList = ui.listViewTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaperTag(id, modelTags.data(modelTags.index(idx.row(), 0)).toInt());
		updateTags();
	}
}

void PaperList::updateTags()
{
	int id = getCurrentPaperID();
	modelTags.setQuery(tr("select ID, Name from Tags \
		where ID in (select Tag from PaperTag where Paper = %1) order by Name").arg(id));
	ui.listViewTags->setModelColumn(1);  // Name
}

void PaperList::onImport()
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

void PaperList::import(const QString& fileName,    const QString& firstHead,
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
		int currentRow;
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

QString PaperList::trimHead(const QString& line, const QString& delimiter) const {
	return line.mid(line.indexOf(delimiter) + delimiter.length());
}

void PaperList::onSetPDF()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		".", tr("PDF file (*.pdf)"));
	if(fileName.isEmpty())
		return;
	QDir(".").mkdir("PDF");
	QString title = modelPapers.data(modelPapers.index(currentRow, PAPER_TITLE)).toString();
	QString pdfFilePath = ".\\PDF\\" + makePDFFileName(title) + ".pdf";
	QFile::copy(fileName, pdfFilePath);
	modelPapers.setData(modelPapers.index(currentRow, PAPER_PDF), pdfFilePath);
}

QString PaperList::makePDFFileName(const QString& title)
{
	QString result = title;
	result.replace(QRegExp("[:|?|*]"), "-");
	return result;
}

void PaperList::onReadPDF()
{
	QString pdfFilePath = modelPapers.data(modelPapers.index(currentRow, PAPER_PDF)).toString();
	QDesktopServices::openUrl(QUrl(pdfFilePath));
}

void PaperList::onSearch(const QString& target)
{
	if(target.isEmpty())
		onCancelSearch();
	else
	{
		QString filter = tr("Title like \'%%1%\' or Authors like \'%%1%\' or Journal like \'%%1%\'").arg(target);
		modelPapers.setFilter(filter);
	}
}

void PaperList::onCancelSearch() 
{
	ui.leSearch->clear();
	modelPapers.setTable("Papers");
	modelPapers.select();
}

void PaperList::onEnableDelTags(const QModelIndex& idx) {
	ui.btDelTag->setEnabled(idx.isValid());
}