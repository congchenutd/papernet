#include "QuotesTableView.h"
#include <QMenu>
#include <QContextMenuEvent>

QuotesTableView::QuotesTableView(QWidget *parent)
	: QTableView(parent) {}

void QuotesTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	QAction actionAddQuote(QIcon(":/MainWindow/Images/Quote.png"),   tr("Add Quote"),    this);
	QAction actionDelQuote(QIcon(":/MainWindow/Images/Uncheck.png"), tr("Delete Quote"), this);

	connect(&actionAddQuote, SIGNAL(triggered()), this, SIGNAL(addQuote()));
	connect(&actionDelQuote, SIGNAL(triggered()), this, SIGNAL(delQuotes()));

	QMenu menu(this);
	menu.addAction(&actionAddQuote);
	menu.addAction(&actionDelQuote);
	menu.exec(event->globalPos());
}

