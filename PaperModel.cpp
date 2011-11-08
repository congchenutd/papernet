#include "PaperModel.h"
#include "PagePapers.h"
#include "Common.h"

PaperModel::PaperModel(QObject *parent) : QSqlTableModel(parent) {}

QVariant PaperModel::data(const QModelIndex& idx, int role) const
{
	if(!idx.isValid())
		return QSqlTableModel::data(idx, role);

	// for related/coauthored
	if(role == Qt::BackgroundRole)
	{
		float maxProximity = getMaxProximity("Papers");
		if(maxProximity > 0)
		{
			float proximity = data(index(idx.row(), PAPER_PROXIMITY)).toInt();
			float red = (maxProximity - proximity) / maxProximity * 55 + 200;
			return QBrush(QColor(red, red, 255));
		}

		float maxCoauthor = getMaxCoauthor();
		if(maxCoauthor > 0)
		{
			float coauthor = data(index(idx.row(), PAPER_COAUTHOR)).toInt();
			float green = (maxCoauthor - coauthor) / maxCoauthor * 55 + 200;
			return QBrush(QColor(255, green, green));
		}
	}

	// make the titles of unread papers bold
	if(idx.column() == PAPER_TITLE)
	{
		if(role == Qt::FontRole && !data(index(idx.row(), PAPER_READ)).toBool())
		{
			QFont f = qApp->font();
			f.setBold(true);
			return f;
		}
	}
	// use icons to represent attachment status
	else if(idx.column() == PAPER_ATTACHED)
	{
		if(role == Qt::DisplayRole)
			return QString();
		if(role == Qt::DecorationRole)
		{
			int paperID = data(index(idx.row(), PAPER_ID), Qt::DisplayRole).toInt();
			AttachmentStatus attachmentStatus = isAttached(paperID);
			if(attachmentStatus == ATTACH_ENDNOTE)
				return QIcon(":/MainWindow/Images/AttachEndNote.png");
			else if(attachmentStatus == ATTACH_PAPER)
				return QIcon(":/MainWindow/Images/AttachPaper.png");
			else if(attachmentStatus == ATTACH_ALL)
				return QIcon(":/MainWindow/Images/AttachAll.png");
		}
	}

	return QSqlTableModel::data(idx, role);
}
