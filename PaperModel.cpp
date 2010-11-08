#include "PaperModel.h"
#include "PagePapers.h"
#include "Common.h"

PaperModel::PaperModel(QObject *parent)
	: QSqlTableModel(parent) {
	icon.addFile(":/MainWindow/Images/Tick.png");
}

QVariant PaperModel::data(const QModelIndex& idx, int role) const
{
	if(!idx.isValid())
		return QSqlTableModel::data(idx, role);

	// for related and coauthor
	if(role == Qt::BackgroundRole)
	{
		float maxProximity = getMaxProximity();
		if(maxProximity > 0)
		{
			float proximity = data(index(idx.row(), PagePapers::PAPER_PROXIMITY)).toInt();
			float red = (maxProximity - proximity)/maxProximity*55 + 200;
			return QBrush(QColor(red, red, 255));
		}

		float maxCoauthor = getMaxCoauthor();
		if(maxCoauthor > 0)
		{
			float coauthor = data(index(idx.row(), PagePapers::PAPER_COAUTHOR)).toInt();
			float green = (maxCoauthor - coauthor)/maxCoauthor*55 + 200;
			return QBrush(QColor(255, green, green));
		}
	}

	// make new paper's title bold
	if(idx.column() == PagePapers::PAPER_TITLE)
	{
		if(role == Qt::FontRole && 
			QSqlTableModel::data(index(idx.row(), PagePapers::PAPER_READ), Qt::DisplayRole).toBool())
		{
			QFont f = qApp->font();
			f.setBold(true);
			return f;
		}
	}
	// tagged
	else if(idx.column() == PagePapers::PAPER_TAGGED)
	{
		if(role == Qt::DisplayRole)
			return QString();
		if(role == Qt::DecorationRole)
		{
			
			//&& QSqlTableModel::data(idx, Qt::DisplayRole).toBool())
			return QIcon(":/MainWindow/Images/Tag.png");
		}
	}
	// attached
	else if(idx.column() == PagePapers::PAPER_ATTACHED)
	{
		if(role == Qt::DisplayRole)
			return QString();
		if(role == Qt::DecorationRole && QSqlTableModel::data(idx, Qt::DisplayRole).toBool())
			return QIcon(":/MainWindow/Images/Attach2.png");
	}

	return QSqlTableModel::data(idx, role);
}