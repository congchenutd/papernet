#include "PageSnippets.h"
#include "AddSnippetDlg.h"
#include "Common.h"
#include <QMessageBox>

PageSnippets::PageSnippets(QWidget *parent)
	: QWidget(parent), currentRow(-1)
{
	ui.setupUi(this);

	resetSnippets();
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(SNIPPET_ID);
	ui.tableView->resizeColumnToContents(SNIPPET_TITLE);

#ifdef Q_WS_MAC
    ui.btAdd   ->setIconSize(QSize(16, 16));
    ui.btDel   ->setIconSize(QSize(16, 16));
#endif

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.btAdd,          SIGNAL(clicked()),     this, SLOT(onAdd()));
	connect(ui.btDel,          SIGNAL(clicked()),     this, SLOT(onDel()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
}

void PageSnippets::onSearch(const QString& target)
{
	if(target.isEmpty())
		resetSnippets();
	else
		model.setFilter(tr("Title   like \"%%1%\" or \
						    Snippet like \"%%1%\" ").arg(target));
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
	dlg.setWindowTitle(tr("Add Snippet"));
	dlg.setSnippetID(getNextID("Snippets", "ID"));
	if(dlg.exec() == QDialog::Accepted)
		model.select();
}

void PageSnippets::onEdit()
{
	AddSnippetDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Snippet"));
	dlg.setSnippetID(getID(currentRow));
	if(dlg.exec() == QDialog::Accepted)
		model.select();
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
	while(model.canFetchMore())
		model.fetchMore();
	ui.tableView->sortByColumn(SNIPPET_TITLE, Qt::AscendingOrder);
}

int PageSnippets::getID(int row) const {
	return model.data(model.index(row, SNIPPET_ID)).toInt();
}
