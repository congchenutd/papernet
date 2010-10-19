#include "PaperTableView.h"
#include <QMenu>
#include <QContextMenuEvent>

PaperTableView::PaperTableView(QWidget *parent)
	: QTableView(parent) {}

void PaperTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	QAction actionShowRelated   (QIcon(":/MainWindow/Images/Related.png"),  tr("Show related"),    this);
	QAction actionShowCoauthored(QIcon(":/MainWindow/Images/Coauthor.png"), tr("Show coauthored"), this);
	connect(&actionShowRelated,    SIGNAL(triggered()), this, SIGNAL(showRelated()));
	connect(&actionShowCoauthored, SIGNAL(triggered()), this, SIGNAL(showCoauthored()));

	QMenu menu(this);
	menu.addAction(&actionShowRelated);
	menu.addAction(&actionShowCoauthored);
	menu.exec(event->globalPos());
}

