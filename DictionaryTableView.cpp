#include "DictionaryTableView.h"
#include <QMenu>
#include <QContextMenuEvent>

DictionaryTableView::DictionaryTableView(QWidget* parent) : AutoSizeTableView(parent)
{}

void DictionaryTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())   // must click on a row
		return;

	QAction actionShowRelated   (QIcon(":/MainWindow/Images/Related.png"),  tr("Show Related"),    this);
	connect(&actionShowRelated,    SIGNAL(triggered()), this, SIGNAL(showRelated()));

	QMenu menu(this);
	menu.addAction(&actionShowRelated);
	menu.exec(event->globalPos());
}