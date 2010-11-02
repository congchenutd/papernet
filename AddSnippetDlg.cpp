#include "AddSnippetDlg.h"
#include "PaperList.h"
#include "Common.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlQuery>

AddSnippetDlg::AddSnippetDlg(QWidget *parent)
	: QDialog(parent), snippetID(-1)
{
	ui.setupUi(this);
	ui.listView->setModel(&model);

	connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.btAdd,    SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(ui.btDel,    SIGNAL(clicked()), this, SLOT(onDel()));
	connect(ui.btSelect, SIGNAL(clicked()), this, SLOT(onSelect()));
}

void AddSnippetDlg::onAdd()
{
	bool ok;
	QString ref = QInputDialog::getText(this, tr("Add Reference"), tr("Reference"), 
							QLineEdit::Normal, QString(""), &ok);
	if(ok && !ref.isEmpty())
		addPaper(ref);
}

void AddSnippetDlg::onDel()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			model.removeRow(idx.row());
	}
}

void AddSnippetDlg::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	ui.btDel->setEnabled(valid);
}

void AddSnippetDlg::accept()
{
	if(ui.teContent->toPlainText().isEmpty())
	{
		ui.teContent->setFocus();
		return;
	}

	QSqlDatabase::database().transaction();
	updateSnippet(snippetID, ui.teContent->toPlainText());
	QStringList list = model.stringList();
	foreach(QString title, list)
	{
		int paperID = getPaperID(title);
		if(paperID == -1)  // not exist
		{
			paperID = getNextID("Papers", "ID");
			::addPaper(paperID, title);
		}
		updatePaperSnippet(paperID, snippetID);
	}
	QSqlDatabase::database().commit();

	return QDialog::accept();
}

void AddSnippetDlg::addPaper(const QString& title)
{
	if(model.stringList().indexOf(title) > -1)
		return;
	int lastRow = model.rowCount();
	model.insertRow(lastRow);
	model.setData(model.index(lastRow, 0), title);
}

void AddSnippetDlg::setSnippet(const QString& snippet) {
	ui.teContent->setPlainText(snippet);
}

void AddSnippetDlg::setSnippetID(int id)
{
	snippetID = id;
	model.setStringList(getPaperList(snippetID));
}

void AddSnippetDlg::onSelect()
{
	PaperList dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{
		QStringList papers = dlg.getSelected();
		foreach(QString paper, papers)
			addPaper(paper);
	}
}

