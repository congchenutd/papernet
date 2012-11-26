#ifndef ATTACHMENTSWIDGET_H
#define ATTACHMENTSWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include "ui_AttachmentsWidget.h"

// widget for attachments
class AttachmentsWidget : public QWidget
{
	Q_OBJECT

public:
	AttachmentsWidget(QWidget *parent = 0);

    void setPaper(int id);   // set the id of the paper
    void update();           // reload

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

    int              _paperID;
    QFileSystemModel _model;
    QModelIndex      _currentIndex;    // index of the selected item in the model
};

#endif // ATTACHMENTSWIDGET_H
