#include "PaperList.h"
#include "common.h"

PaperList::PaperList(QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);

	model.setTable("Papers");
	model.select();

	ui.tableView->setModel(&model);
	ui.tableView->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
    for(int col = PAPER_ID; col <= PAPER_NOTE; ++col)
        ui.tableView->hideColumn(col);
    ui.tableView->showColumn(PAPER_TITLE);
    ui.tableView->showColumn(PAPER_AUTHORS);
    ui.tableView->showColumn(PAPER_YEAR);
	ui.tableView->resizeColumnToContents(PAPER_TITLE);
    ui.buttonBox->setEnabled(false);

	connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
    connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection)));
}

QStringList PaperList::getSelected() const
{
	QStringList result;
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		result << model.data(model.index(idx.row(), PAPER_TITLE)).toString();
    return result;
}

void PaperList::onSelectionChanged(const QItemSelection& selected) {
    ui.buttonBox->setEnabled(!selected.isEmpty());
}

void PaperList::onSearch(const QString& target)
{
	if(target.isEmpty())
		model.select();
	else
		model.setFilter(
            tr("Title       like \"%%1%\" or \
                Authors     like \"%%1%\" or \
                Year        like \"%%1%\" or \
                Publication like \"%%1%\" or \
                Abstract    like \"%%1%\" or \
                Note        like \"%%1%\" ").arg(target));
}
