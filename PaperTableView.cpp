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
	connect(&actionShowRelated,    SIGNAL(triggered()), this, SIGNAL(showRelated()));
	connect(&actionShowCoauthored, SIGNAL(triggered()), this, SIGNAL(showCoauthored()));
	connect(&actionAddQuote,       SIGNAL(triggered()), this, SIGNAL(addQuote()));

	QMenu menu(this);
	menu.addAction(&actionShowRelated);
	menu.addAction(&actionShowCoauthored);
	menu.addAction(&actionAddQuote);
	menu.exec(event->globalPos());
}
