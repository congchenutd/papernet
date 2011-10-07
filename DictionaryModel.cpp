#include "DictionaryModel.h"
#include "Common.h"
#include <QBrush>

DictionaryModel::DictionaryModel(QObject *parent) : QSqlTableModel(parent) {}

QVariant DictionaryModel::data(const QModelIndex& idx, int role) const
{
	if(!idx.isValid())
		return QSqlTableModel::data(idx, role);

	// for related
	if(role == Qt::BackgroundRole)
	{
		float maxProximity = getMaxProximity("Dictionary");
		if(maxProximity > 0)
		{
			float proximity = data(index(idx.row(), DICTIONARY_PROXIMITY)).toInt();
			float red = (maxProximity - proximity) / maxProximity * 55 + 200;
			return QBrush(QColor(red, red, 255));
		}
	}

	return QSqlTableModel::data(idx, role);
}