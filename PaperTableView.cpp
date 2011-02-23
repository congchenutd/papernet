#include "PaperTableView.h"
#include <QMenu>
#include <QContextMenuEvent>

PaperTableView::PaperTableView(QWidget *parent)
	: AutoSizeTableView(parent) {}

void PaperTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	QAction actionShowRelated   (QIcon(":/MainWindow/Images/Related.png"),  tr("Show related"),    this);
	QAction actionShowCoauthored(QIcon(":/MainWindow/Images/Coauthor.png"), tr("Show coauthored"), this);
	QAction actionAddSnippet    (QIcon(":/MainWindow/Images/Snippet.png"),  tr("Add snippet"), this);
	connect(&actionShowRelated,    SIGNAL(triggered()), this, SIGNAL(showRelated()));
	connect(&actionShowCoauthored, SIGNAL(triggered()), this, SIGNAL(showCoauthored()));
	connect(&actionAddSnippet,     SIGNAL(triggered()), this, SIGNAL(addSnippet()));

	QMenu menu(this);
	menu.addAction(&actionShowRelated);
	menu.addAction(&actionShowCoauthored);
	menu.addAction(&actionAddSnippet);
	menu.exec(event->globalPos());
}
