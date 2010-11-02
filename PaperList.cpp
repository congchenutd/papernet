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
	ui.tableView->resizeColumnToContents(PagePapers::PAPER_TITLE);

	connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
}

QStringList PaperList::getSelected() const
{
	QStringList result;
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	foreach(QModelIndex idx, idxList)
		result << model.data(model.index(idx.row(), PagePapers::PAPER_TITLE)).toString();
	return result;
}

void PaperList::onSearch(const QString& target)
{
	if(target.isEmpty())
		model.select();
	else
		model.setFilter(
			tr("Title    like \'%%1%\' or \
				Authors  like \'%%1%\' or \
				Year     like \'%%1%\' or \
				Journal  like \'%%1%\' or \
				Abstract like \'%%1%\' or \
				Note     like \'%%1%\' ").arg(target));
}

