#include "AddSnippetDlg.h"
#include "Common.h"
#include <QInputDialog>
#include <QMessageBox>

AddSnippetDlg::AddSnippetDlg(QWidget *parent)
	: QDialog(parent), paperID(-1), snippetID(-1)
{
	ui.setupUi(this);
	ui.listView->setModel(&model);

	connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.btAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(ui.btDel, SIGNAL(clicked()), this, SLOT(onDel()));
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
		{
			model.removeRow(idx.row());
		}
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
	QStringList list = model.stringList();
	foreach(QString ref, list)
	{

	}
	return QDialog::accept();
}

void AddSnippetDlg::setPaperID(int paper)
{
	paperID = paper;
	addPaper(getPaperTitle(paperID));
}

void AddSnippetDlg::addPaper(const QString& title)
{
	int lastRow = model.rowCount();
	model.insertRow(lastRow);
	model.setData(model.index(lastRow, 0), title);
}