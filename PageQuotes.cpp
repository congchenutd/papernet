#include "PageQuotes.h"
#include "AddQuoteDlg.h"
#include "Common.h"
#include "Navigator.h"
#include <QMessageBox>

PageQuotes::PageQuotes(QWidget *parent)
	: Page(parent)
{
	currentRow = -1;
	ui.setupUi(this);
	ui.tableView->init("PageQuotes");   // set the table name for the view

	resetQuotes();   // init model
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(QUOTE_ID);
	ui.tableView->resizeColumnToContents(QUOTE_TITLE);

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
	connect(ui.tableView, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
}

void PageQuotes::search(const QString& target)
{
	if(!target.isEmpty())
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

void PageQuotes::onClicked(const QModelIndex& idx) {
	Navigator::getInstance()->addFootStep(this, getID(idx.row()));  // track navigation
}

void PageQuotes::add()
{
	reset();
	AddQuoteDlg* dlg = new AddQuoteDlg(this);    // non-modal
	connect(dlg, SIGNAL(accepted()), this, SLOT(onAccepted()));
	dlg->setWindowTitle(tr("Add Quote"));
	dlg->setQuoteID(getNextID("Quotes", "ID"));
	dlg->show();
}

void PageQuotes::onEdit()
{
	AddQuoteDlg* dlg = new AddQuoteDlg(this);    // non-modal
	connect(dlg, SIGNAL(accepted()), this, SLOT(onAccepted()));
	dlg->setWindowTitle(tr("Edit Quote"));
	dlg->setQuoteID(getID(currentRow));
	dlg->show();
}

void PageQuotes::del()
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

void PageQuotes::jumpToID(int id)
{
	int row = idToRow(&model, QUOTE_ID, id);
	if(row > -1)
	{
		currentRow = row;
		ui.tableView->selectRow(currentRow);  // will trigger onCurrentRowChanged()
		ui.tableView->setFocus();
	}
}
