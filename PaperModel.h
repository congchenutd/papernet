#ifndef PAPERMODEL_H
#define PAPERMODEL_H

#include <QSqlTableModel>
#include <QStandardItemModel>
#include <QIcon>

// A table model for papers
class PaperModel : public QSqlTableModel
{
public:
	PaperModel(QObject* parent = 0);
	QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const;
};

// an in-mem cache for QSqlTableModel
class InMemTableModel : public QStandardItemModel
{
public:
    InMemTableModel(QObject* parent = 0);
    void setTable(const QString& table);
    void select();
    void setFilter(const QString& filter);
    QString tableName() const;
    bool submit();

private:
    QSqlTableModel* _rawModel;
};

#endif // PAPERMODEL_H
