#ifndef PAGEDICTIONARY_H
#define PAGEDICTIONARY_H

#include <QWidget>
#include "ui_PageDictionary.h"
#include <QSqlTableModel>

class PageDictionary : public QWidget
{
	Q_OBJECT

public:
	PageDictionary(QWidget *parent = 0);

private:
	Ui::PageDictionary ui;
	QSqlTableModel model;
};

#endif // PAGEDICTIONARY_H
