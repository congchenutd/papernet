#include "AddPaperTagDlg.h"
#include <QtGui>

AddPaperTagDlg::AddPaperTagDlg(QWidget *parent, const QString& tableName)
	: QDialog(parent)
{
	ui.setupUi(this);
	model.setTable(tableName);
	model.select();
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(0);  // ID
	ui.tableView->horizontalHeader()->setStretchLastSection(true);
}

QList<int> AddPaperTagDlg::getSelected() const
{
	QItemSelectionModel* selectionModel = ui.tableView->selectionModel();
	QModelIndexList idxList = selectionModel->selectedRows();
	QList<int> result;
	foreach(QModelIndex idx, idxList)
		result << model.data(model.index(idx.row(), 0)).toInt();
	return result;
}
