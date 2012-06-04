#ifndef ATTACHMENTSWIDGET_H
#define ATTACHMENTSWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include "ui_AttachmentsWidget.h"

class AttachmentsWidget : public QWidget
{
	Q_OBJECT

public:
	AttachmentsWidget(QWidget *parent = 0);

	void setPaper(int id);
	void update();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void showEvent(QShowEvent*);

public slots:
	void onAddFile();
private slots:
	void onAddLink();
	void onRename();
	void onDel();
	void onOpen(const QModelIndex& idx);

private:
	Ui::AttachmentsWidgetClass ui;

	int paperID;
	QFileSystemModel model;
	QModelIndex currentIndex;
};

#endif // ATTACHMENTSWIDGET_H
