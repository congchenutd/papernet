#ifndef RELATEDPAPERSWIDGET_H
#define RELATEDPAPERSWIDGET_H

#include "ui_RelatedPapersWidget.h"
#include <QStandardItemModel>

class RelatedPapersWidget : public QWidget
{
	Q_OBJECT
	
public:
	RelatedPapersWidget(QWidget* parent = 0);
    void setCentralPaper(int paperID);

protected:
    void showEvent(QShowEvent*);

private slots:
	void onRelatedDoubleClicked(const QModelIndex& idx);

private:
    void update();

signals:
	void doubleClicked(int);

private:
	Ui::RelatedPapersWidget ui;
    int centralPaperID;
	QStandardItemModel model;

	enum {COL_ID, COL_TITLE, COL_PROXIMITY};
};

#endif // RELATEDPAPERSWIDGET_H
