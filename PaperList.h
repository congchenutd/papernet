#ifndef PAPERLIST_H
#define PAPERLIST_H

#include <QDialog>
#include <QSqlTableModel>
#include "ui_PaperList.h"

// A read-only list of papers
class PaperList : public QDialog
{
	Q_OBJECT

public:
	PaperList(QWidget *parent = 0);
	QStringList getSelected() const;

private slots:
	void onSearch(const QString& target);

private:
	Ui::PaperListClass ui;
	QSqlTableModel model;
};

#endif // PAPERLIST_H
