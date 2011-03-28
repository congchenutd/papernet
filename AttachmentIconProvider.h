#ifndef ATTACHMENTICONPROVIDER_H
#define ATTACHMENTICONPROVIDER_H

#include <QFileIconProvider>

class AttachmentIconProvider : public QFileIconProvider
{
public:
	AttachmentIconProvider();
	virtual QIcon icon(const QFileInfo& info) const;
};

#endif // ATTACHMENTICONPROVIDER_H
