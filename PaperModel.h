#ifndef PAPERMODEL_H
#define PAPERMODEL_H

#include <QSqlTableModel>
#include <QIcon>

// A table model for papers
class PaperModel : public QSqlTableModel
{
	Q_OBJECT

public:
	PaperModel(QObject* parent = 0);
	QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const;
};

#endif // PAPERMODEL_H
