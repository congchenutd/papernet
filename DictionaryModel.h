#ifndef DICTIONARYMODEL_H
#define DICTIONARYMODEL_H

#include <QSqlTableModel>

// A table model for dictionary
// Show related phrase in gradient color
class DictionaryModel : public QSqlTableModel
{
	Q_OBJECT

public:
	DictionaryModel(QObject* parent = 0);
	QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const;
};

#endif // DICTIONARYMODEL_H
