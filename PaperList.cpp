#include "PaperList.h"
#include "PagePapers.h"

PaperList::PaperList(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	model.setTable("Papers");
	model.select();

	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(PagePapers::PAPER_ID);
	ui.tableView->hideColumn(PagePapers::PAPER_JOURNAL);
	ui.tableView->hideColumn(PagePapers::PAPER_ABSTRACT);
	ui.tableView->hideColumn(PagePapers::PAPER_NOTE);
	ui.tableView->hideColumn(PagePapers::PAPER_PROXIMITY);
	ui.tableView->hideColumn(PagePapers::PAPER_COAUTHOR);
}

QStringList PaperList::getSelected() const
{
	QStringList result;
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		result << model.data(model.index(idx.row(), PagePapers::PAPER_TITLE)).toString();
	return result;
}

