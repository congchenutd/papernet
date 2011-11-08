#include "PaperList.h"
#include "common.h"

PaperList::PaperList(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	model.setTable("Papers");
	model.select();

	ui.tableView->setModel(&model);
	ui.tableView->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);
	ui.tableView->hideColumn(PAPER_ID);
	ui.tableView->hideColumn(PAPER_JOURNAL);
	ui.tableView->hideColumn(PAPER_ABSTRACT);
	ui.tableView->hideColumn(PAPER_NOTE);
	ui.tableView->hideColumn(PAPER_PROXIMITY);
	ui.tableView->hideColumn(PAPER_COAUTHOR);
	ui.tableView->hideColumn(PAPER_READ);
	ui.tableView->hideColumn(PAPER_ATTACHED);
	ui.tableView->resizeColumnToContents(PAPER_TITLE);

	connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
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
		model.select();
	else
		model.setFilter(
			tr("Title    like \"%%1%\" or \
				Authors  like \"%%1%\" or \
				Year     like \"%%1%\" or \
				Journal  like \"%%1%\" or \
				Abstract like \"%%1%\" or \
				Note     like \"%%1%\" ").arg(target));
}
