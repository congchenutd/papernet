#include "ModelPapers.h"

ModelPapers::ModelPapers(QObject *parent)
	: QSqlTableModel(parent)
{

}

QVariant ModelPapers::data(const QModelIndex& idx, int role) const
{
	if(role == Qt::DecorationRole && idx.isValid())
	{

	}
	return QSqlTableModel::data(idx, role);
}

void ModelPapers::setCentral(int id) {
	centralPaperID = id;
}
