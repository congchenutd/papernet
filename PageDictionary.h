#ifndef PAGEDICTIONARY_H
#define PAGEDICTIONARY_H

#include <QWidget>
#include "ui_PageDictionary.h"

class PageDictionary : public QWidget
{
	Q_OBJECT

public:
	PageDictionary(QWidget *parent = 0);
	~PageDictionary();

private:
	Ui::PageDictionary ui;
};

#endif // PAGEDICTIONARY_H
