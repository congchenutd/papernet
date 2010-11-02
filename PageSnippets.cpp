#include "PageSnippets.h"
#include "AddSnippetDlg.h"
#include "Common.h"
#include <QMessageBox>

PageSnippets::PageSnippets(QWidget *parent)
	: QWidget(parent), currentRow(-1)
{
	ui.setupUi(this);
	onShowSearch(false);

	model.setEditStrategy(QSqlTableModel::OnManualSubmit);
	resetSnippets();
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(SNIPPET_ID);

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.btAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(ui.btDel, SIGNAL(clicked()), this, SLOT(onDel()));
	connect(ui.btSearch, SIGNAL(toggled(bool)),       this, SLOT(onShowSearch(bool)));
	connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
}

void PageSnippets::onShowSearch(bool enable)
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
//		resetPapers();
	}
}

void PageSnippets::onSearch(const QString& target)
{

}

void PageSnippets::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRow = valid ? idxList.front().row() : -1;	
	ui.btDel->setEnabled(valid);
}

void PageSnippets::onAdd()
{
	AddSnippetDlg dlg(this);
	dlg.setSnippetID(getNextID("Snippets", "ID"));
	if(dlg.exec() == QDialog::Accepted)
	{
		model.select();
	}
}

void PageSnippets::onDel()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delSnippet(getID(idx.row()));
		model.select();
	}
}

void PageSnippets::resetSnippets()
{
	model.setTable("Snippets");
	model.select();
	ui.tableView->sortByColumn(SNIPPET_SNIPPET, Qt::AscendingOrder);
}

void PageSnippets::onEdit()
{
	AddSnippetDlg dlg(this);
	dlg.setSnippetID(getID(currentRow));
	dlg.setSnippet(model.data(model.index(currentRow, SNIPPET_SNIPPET)).toString());
	if(dlg.exec() == QDialog::Accepted)
	{
		model.select();
	}
}

int PageSnippets::getID(int row) const {
	return model.data(model.index(row, SNIPPET_ID)).toInt();
}

void PageSnippets::enter() {
	model.select();
}
