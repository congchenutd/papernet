#include "PaperTableView.h"
#include <QMenu>
#include <QContextMenuEvent>

PaperTableView::PaperTableView(QWidget *parent)
	: AutoSizeTableView(parent) {}

void PaperTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	QAction actionShowRelated   (QIcon(":/MainWindow/Images/Related.png"),  tr("Show Related"),    this);
	QAction actionShowCoauthored(QIcon(":/MainWindow/Images/Coauthor.png"), tr("Show Coauthored"), this);
	QAction actionAddQuote      (QIcon(":/MainWindow/Images/Quote.png"),    tr("Add Quote"),       this);
	QAction actionPrintMe       (QIcon(":/MainWindow/Images/Print.png"),    tr("Print me"),        this);
	QAction actionReadMe        (QIcon(":/MainWindow/Images/Read.png"),     tr("Read me"),        this);
	actionReadMe.setCheckable(true);

	connect(&actionShowRelated,    SIGNAL(triggered()),     this, SIGNAL(showRelated()));
	connect(&actionShowCoauthored, SIGNAL(triggered()),     this, SIGNAL(showCoauthored()));
	connect(&actionAddQuote,       SIGNAL(triggered()),     this, SIGNAL(addQuote()));
	connect(&actionPrintMe,        SIGNAL(triggered()),     this, SIGNAL(printMe()));
	connect(&actionReadMe,         SIGNAL(triggered(bool)), this, SIGNAL(readMe(bool)));

	QMenu menu(this);
	menu.addAction(&actionShowRelated);
	menu.addAction(&actionShowCoauthored);
	menu.addAction(&actionAddQuote);
	menu.addAction(&actionPrintMe);
	menu.addAction(&actionReadMe);
	menu.exec(event->globalPos());
}
