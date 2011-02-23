#include "PageSnippets.h"
#include "AddSnippetDlg.h"
#include "Common.h"
#include <QMessageBox>

PageSnippets::PageSnippets(QWidget *parent)
	: QWidget(parent), currentRow(-1)
{
	ui.setupUi(this);
	ui.tableView->init("PageSnippets");

	resetSnippets();
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(SNIPPET_ID);
	ui.tableView->resizeColumnToContents(SNIPPET_TITLE);

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
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
	emit tableValid(valid);
}

void PageSnippets::onAdd()
{
	AddSnippetDlg* dlg = new AddSnippetDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onAccepted()));
	dlg->setWindowTitle(tr("Add Snippet"));
	dlg->setSnippetID(getNextID("Snippets", "ID"));
	dlg->show();
}

void PageSnippets::onEdit()
{
	AddSnippetDlg* dlg = new AddSnippetDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onAccepted()));
	dlg->setWindowTitle(tr("Edit Snippet"));
	dlg->setSnippetID(getID(currentRow));
	dlg->show();
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

void PageSnippets::onAccepted() {
	model.select();
}

void PageSnippets::jumpToSnippet(int snippetID)
{
	QModelIndexList indexes = model.match(
		model.index(0, SNIPPET_ID), Qt::DisplayRole, snippetID, 1, Qt::MatchExactly | Qt::MatchWrap);
	if(!indexes.isEmpty())
	{
		ui.tableView->selectRow(indexes.at(0).row());
		ui.tableView->setFocus();
	}
}

void PageSnippets::saveSectionSizes() {
	ui.tableView->saveSectionSizes();
}