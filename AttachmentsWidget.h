#ifndef ATTACHMENTSWIDGET_H
#define ATTACHMENTSWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include "ui_AttachmentsWidget.h"

class AttachmentIconProvider;

class AttachmentsWidget : public QWidget
{
	Q_OBJECT

public:
	AttachmentsWidget(QWidget *parent = 0);
	~AttachmentsWidget();

	void setPaper(int id);
	void update();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void onAddFile();
	void onAddLink();
	void onRename();
	void onDel();
	void onOpen(const QModelIndex& idx);

private:
	QString suggestName(const QString &fileName);

signals:
	void paperRead();

private:
	Ui::AttachmentsWidgetClass ui;

	int paperID;
	QFileSystemModel model;
	QModelIndex currentIndex;
	AttachmentIconProvider* iconProvider;
};

class AttachmentIconProvider : public QFileIconProvider
{
public:
	AttachmentIconProvider();
	virtual QIcon icon(const QFileInfo& info) const;
};

#endif // ATTACHMENTSWIDGET_H
