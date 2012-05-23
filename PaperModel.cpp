#include "PaperModel.h"
#include "PagePapers.h"
#include "Common.h"

PaperModel::PaperModel(QObject *parent) : QSqlTableModel(parent) {}

QVariant PaperModel::data(const QModelIndex& idx, int role) const
{
	if(!idx.isValid())
		return QSqlTableModel::data(idx, role);

	// make the titles of unread papers bold
	if(idx.column() == PAPER_TITLE)
	{
		if(role == Qt::FontRole && !isPaperRead(data(index(idx.row(), PAPER_ID)).toInt()) )
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
				return QIcon(":/Images/AttachEndNote.png");
			else if(attachmentStatus == ATTACH_PAPER)
				return QIcon(":/Images/AttachPaper.png");
			else if(attachmentStatus == ATTACH_ALL)
				return QIcon(":/Images/AttachAll.png");
		}
	}

	return QSqlTableModel::data(idx, role);
}
