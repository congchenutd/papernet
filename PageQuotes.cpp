#include "PageQuotes.h"
#include "AddQuoteDlg.h"
#include "Common.h"
#include <QMessageBox>

PageQuotes::PageQuotes(QWidget *parent)
	: QWidget(parent)
{
	currentRow = -1;
	ui.setupUi(this);
	ui.tableView->init("PageQuotes");

	resetQuotes();
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(QUOTE_ID);
	ui.tableView->resizeColumnToContents(QUOTE_TITLE);

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
}

void PageQuotes::onSearch(const QString& target)
{
	if(target.isEmpty())
		resetQuotes();
	else
		model.setFilter(tr("Title like \"%%1%\" or \
							Quote like \"%%1%\" ").arg(target));
}

void PageQuotes::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRow = valid ? idxList.front().row() : -1;
	emit tableValid(valid);
}

void PageQuotes::onAdd()
{
	AddQuoteDlg* dlg = new AddQuoteDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onAccepted()));
	dlg->setWindowTitle(tr("Add Quote"));
	dlg->setQuoteID(getNextID("Quotes", "ID"));
	dlg->show();
}

void PageQuotes::onEdit()
{
	AddQuoteDlg* dlg = new AddQuoteDlg(this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(onAccepted()));
	dlg->setWindowTitle(tr("Edit Quote"));
	dlg->setQuoteID(getID(currentRow));
	dlg->show();
}

void PageQuotes::onDel()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delQuote(getID(idx.row()));
		model.select();
	}
}

void PageQuotes::resetQuotes()
{
	model.setTable("Quotes");
	model.select();
	while(model.canFetchMore())
		model.fetchMore();
	ui.tableView->sortByColumn(QUOTE_TITLE, Qt::AscendingOrder);
}

int PageQuotes::getID(int row) const {
	return model.data(model.index(row, QUOTE_ID)).toInt();
}

void PageQuotes::onAccepted() {
	model.select();
}

void PageQuotes::jumpToQuote(int quoteID)
{
	QModelIndexList indexes = model.match(
		model.index(0, QUOTE_ID), Qt::DisplayRole, quoteID, 1, Qt::MatchExactly | Qt::MatchWrap);
	if(!indexes.isEmpty())
	{
		ui.tableView->selectRow(indexes.at(0).row());
		ui.tableView->setFocus();
	}
}
