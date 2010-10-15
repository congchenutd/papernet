#ifndef MODELPAPERS_H
#define MODELPAPERS_H

#include <QSqlTableModel>

class ModelPapers : public QSqlTableModel
{
	Q_OBJECT

public:
	ModelPapers(QObject* parent = 0);
	void setCentral(int id);

	virtual QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const;

private:
	int centralPaperID;
};

#endif // MODELPAPERS_H
