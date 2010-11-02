#ifndef PAGESNIPPETS_H
#define PAGESNIPPETS_H

#include <QWidget>
#include "ui_PageSnippets.h"

class PageSnippets : public QWidget
{
	Q_OBJECT

public:
	PageSnippets(QWidget *parent = 0);
	~PageSnippets();

private:
	Ui::PageSnippetsClass ui;
};

#endif // PAGESNIPPETS_H
