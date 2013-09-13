#include "PaperModel.h"
#include "PagePapers.h"
#include "Common.h"

PaperModel::PaperModel(QObject *parent) : QSqlTableModel(parent) {}

QVariant PaperModel::data(const QModelIndex& idx, int role) const
{
	if(!idx.isValid())
		return QSqlTableModel::data(idx, role);

	if(idx.column() == PAPER_TITLE)
	{
        int paperID = data(index(idx.row(), PAPER_ID), Qt::DisplayRole).toInt();

        // make the titles of unread papers bold
        if(role == Qt::FontRole && !isPaperRead(paperID))
		{
			QFont f = qApp->font();
			f.setBold(true);
			return f;
		}
        // attachment icon
        else if(role == Qt::DecorationRole && pdfAttached(paperID))
            return QIcon(":/Images/AttachPaper.png");
	}

	return QSqlTableModel::data(idx, role);
}


////////////////////////////////////////////////////////////////////////////
InMemTableModel::InMemTableModel(QObject* parent)
    : QStandardItemModel(parent),
      _rawModel(new QSqlTableModel(this))
{}

void InMemTableModel::setTable(const QString& table) {
    _rawModel->setTable(table);
}

void InMemTableModel::select()
{
    // let the tablemodel load all
    _rawModel->select();
    while(_rawModel->canFetchMore())
        _rawModel->fetchMore();

    // copy data
    removeRows(0, rowCount());
    setColumnCount(_rawModel->columnCount());
    setRowCount   (_rawModel->rowCount());
    for(int row = 0; row < rowCount(); ++row)
        for(int col = 0; col < columnCount(); ++col)
            setData(index(row, col), _rawModel->data(_rawModel->index(row, col)));
}

void InMemTableModel::setFilter(const QString& filter)
{
    if(filter.isEmpty())
        setTable(tableName());
    else
        _rawModel->setFilter(filter);
    select();
}

QString InMemTableModel::tableName() const {
    return _rawModel->tableName();
}

bool InMemTableModel::submit() {
    return _rawModel->submitAll();
}
