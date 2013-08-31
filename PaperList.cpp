#include "PaperList.h"
#include "Common.h"
#include <QLabel>

PaperList::PaperList(QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);
    resize(1100, 500);

    resetModel();

	ui.tableView->setModel(&model);
    ui.tableView->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
    for(int col = PAPER_ID; col <= PAPER_NOTE; ++col)
        ui.tableView->hideColumn(col);
    ui.tableView->showColumn(PAPER_TITLE);
    ui.tableView->showColumn(PAPER_AUTHORS);
    ui.tableView->showColumn(PAPER_YEAR);
    ui.tableView->resizeColumnToContents(PAPER_TITLE);

    // search line edit
    QLabel* label = new QLabel;
    label->setPixmap(QPixmap(":/Images/Search.png"));
    label->resize(16, 16);

    ClearButton* btClear = new ClearButton;
    btClear->setToolTip(tr("Clear"));
    btClear->resize(16, 16);

    ui.leSearch->setLabel      (label);
    ui.leSearch->setClearButton(btClear);

    connect(ui.leSearch,  SIGNAL(filter(QString)),            this, SLOT(onSearch(QString)));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
}

QStringList PaperList::getSelected() const
{
	QStringList result;
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		result << model.data(model.index(idx.row(), PAPER_TITLE)).toString();
    return result;
}

void PaperList::onSearch(const QString& target)
{
	if(target.isEmpty())
        resetModel();
    else
		model.setFilter(
            tr("Title       like \"%%1%\" or \
                Authors     like \"%%1%\" or \
                Year        like \"%%1%\" or \
                Publication like \"%%1%\" or \
                Abstract    like \"%%1%\" or \
               Note        like \"%%1%\" ").arg(target));
}

void PaperList::resetModel()
{
    model.setTable("Papers");
    model.select();
    ui.tableView->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
}
