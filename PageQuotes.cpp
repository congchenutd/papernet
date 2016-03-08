#include "PageQuotes.h"
#include "QuoteDlg.h"
#include "Common.h"
#include "Navigator.h"
#include "OptionDlg.h"
#include <QMessageBox>

PageQuotes::PageQuotes(QWidget *parent)
	: Page(parent)
{
	_currentRow = -1;
	ui.setupUi(this);
	ui.tableView->init("PageQuotes", UserSetting::getInstance());

	reset();   // init model
	ui.tableView->setModel(&_model);

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this, SLOT(onSelectionChanged(QItemSelection)));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
    connect(ui.tableView, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
}

void PageQuotes::saveGeometry() {
    ui.tableView->saveSectionSizes();
}

void PageQuotes::search(const QString& target)
{
	if(!target.isEmpty())
		_model.setFilter(tr("Title like \"%%1%\" or \
                             Quote like \"%%1%\" ").arg(target));
}

void PageQuotes::onSelectionChanged(const QItemSelection& selected)
{
	if(!selected.isEmpty())
		_currentRow = selected.indexes().front().row();
	emit selectionValid(!selected.isEmpty());
}

void PageQuotes::onClicked(const QModelIndex& idx) {
	Navigator::getInstance()->addFootStep(this, rowToID(idx.row()));  // track navigation
}

void PageQuotes::addRecord()
{
	QuoteDlg dlg(this);
	dlg.setWindowTitle(tr("Add Quote"));
	dlg.setQuoteID(getNextID("Quotes", "ID"));
	if(dlg.exec() == QDialog::Accepted)
		reset();   // just refresh, dlg will submit the changes
}

void PageQuotes::onEdit()
{
	QuoteDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Quote"));
	dlg.setQuoteID(rowToID(_currentRow));
	if(dlg.exec() == QDialog::Accepted)
		reset();   // just refresh, dlg will submit the changes
}

void PageQuotes::delRecord()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delQuote(rowToID(idx.row()));
		reset();   // just refresh, dlg will submit the changes
	}
}

void PageQuotes::editRecord() {
	onEdit();
}

void PageQuotes::reset()
{
    _model.setTable("Quotes");
    _model.select();
    fetchAll(&_model);
    ui.tableView->hideColumn(QUOTE_ID);
    ui.tableView->sortByColumn(QUOTE_TITLE, Qt::AscendingOrder);
    ui.tableView->loadSectionSizes();
}

int PageQuotes::rowToID(int row) const {
	return _model.data(_model.index(row, QUOTE_ID)).toInt();
}

void PageQuotes::jumpToID(int id)
{
	fetchAll(&_model);
	int row = idToRow(&_model, QUOTE_ID, id);
	if(row > -1)
	{
		_currentRow = row;
		ui.tableView->selectRow(_currentRow);  // will trigger onSelectionChanged()
		ui.tableView->setFocus();
	}
}
