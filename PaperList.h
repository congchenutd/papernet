#ifndef PAPERLIST_H
#define PAPERLIST_H

#include <QDialog>
#include <QSqlTableModel>
#include "ui_PaperList.h"

class PaperList : public QDialog
{
	Q_OBJECT

public:
	PaperList(QWidget *parent = 0);
	QStringList getSelected() const;

private:
	Ui::PaperListClass ui;
	QSqlTableModel model;
};

#endif // PAPERLIST_H
