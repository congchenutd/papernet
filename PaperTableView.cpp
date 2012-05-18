#include "PaperTableView.h"
#include "Common.h"
#include <QMenu>
#include <QContextMenuEvent>

PaperTableView::PaperTableView(QWidget *parent)
	: AutoSizeTableView(parent)
{
	actionAddQuote = new QAction(QIcon(":/MainWindow/Images/Quote.png"), tr("Add Quote"), this);
	actionPrintMe  = new QAction(QIcon(":/MainWindow/Images/Print.png"), tr("Print me"),  this);
	actionReadMe   = new QAction(QIcon(":/MainWindow/Images/Read.png"),  tr("Read me"),   this);
	actionPrintMe->setCheckable(true);
	actionReadMe->setCheckable(true);

	connect(actionAddQuote, SIGNAL(triggered()),     this, SIGNAL(addQuote()));
	connect(actionPrintMe,  SIGNAL(triggered(bool)), this, SIGNAL(printMe(bool)));
	connect(actionReadMe,   SIGNAL(triggered(bool)), this, SIGNAL(readMe(bool)));
}

void PaperTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	int paperID = model()->data(model()->index(rowAt(event->pos().y()), PAPER_ID)).toInt();
	actionPrintMe->setChecked(isPaperToBePrinted(paperID));
	actionReadMe->setChecked(!isPaperRead(paperID));

	QMenu menu(this);
	menu.addAction(actionAddQuote);
	menu.addAction(actionPrintMe);
	menu.addAction(actionReadMe);
	menu.exec(event->globalPos());
}
