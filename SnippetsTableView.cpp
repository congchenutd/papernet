#include "SnippetsTableView.h"
#include <QMenu>
#include <QContextMenuEvent>

SnippetsTableView::SnippetsTableView(QWidget *parent)
	: QTableView(parent) {}

void SnippetsTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	QAction actionAddSnippet(QIcon(":/MainWindow/Images/Snippet.png"),  tr("Add snippet"), this);
	QAction actionDelSnippet(QIcon(":/MainWindow/Images/Uncheck.png"),  tr("Delete snippet"), this);
	
	connect(&actionAddSnippet, SIGNAL(triggered()), this, SIGNAL(addSnippet()));
	connect(&actionDelSnippet, SIGNAL(triggered()), this, SIGNAL(delSnippets()));

	QMenu menu(this);
	menu.addAction(&actionAddSnippet);
	menu.addAction(&actionDelSnippet);
	menu.exec(event->globalPos());
}

