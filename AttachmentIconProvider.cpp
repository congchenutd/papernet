#include "AttachmentIconProvider.h"

AttachmentIconProvider::AttachmentIconProvider()
	: QFileIconProvider()
{}

QIcon AttachmentIconProvider::icon(const QFileInfo& info) const
{
	if(info.fileName().compare("Paper.pdf", Qt::CaseInsensitive) == 0)
		return QIcon(":/MainWindow/Images/PDF.png");
	return QFileIconProvider::icon(info);
}
