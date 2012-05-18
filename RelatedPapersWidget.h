#ifndef RELATEDPAPERSWIDGET_H
#define RELATEDPAPERSWIDGET_H

#include "ui_RelatedPapersWidget.h"
#include <QStandardItemModel>

class Thesaurus;

class RelatedPapersWidget : public QWidget
{
	Q_OBJECT
	
public:
	RelatedPapersWidget(QWidget* parent = 0);
	void setCentralPaper(int id);

private slots:
	void onThesaurus(const QStringList& relatedTags);
	void onPaperDoubleClicked(const QModelIndex& idx);

private:
	void sort();

signals:
	void doubleClicked(int);
	
private:
	Ui::RelatedPapersWidget ui;
	Thesaurus* thesaurus;
	int paperID;
	QStandardItemModel model;

	enum {COL_ID, COL_TITLE, COL_PROXIMITY};
};

#endif // RELATEDPAPERSWIDGET_H
