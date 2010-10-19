#include "PaperModel.h"
#include "PagePapers.h"
#include "Common.h"

PaperModel::PaperModel(QObject *parent)
	: QSqlTableModel(parent) {
	icon.addFile(":/MainWindow/Images/Tick.png");
}

QVariant PaperModel::data(const QModelIndex& idx, int role) const
{
	if(role == Qt::BackgroundRole && idx.isValid())
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
	return QSqlTableModel::data(idx, role);
}