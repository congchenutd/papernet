#include "PaperTableView.h"
#include "Common.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>

PaperTableView::PaperTableView(QWidget *parent)
	: AutoSizeTableView(parent)
{
	actionAddQuote = new QAction(QIcon(":/Images/Quote.png"),    tr("Add Quote"), this);
	actionPrintMe  = new QAction(QIcon(":/Images/Print.png"),    tr("Print me"),  this);
	actionBookmark = new QAction(QIcon(":/Images/Bookmark.png"), tr("Bookmark"),   this);
	actionAddPDF   = new QAction(QIcon(":/Images/PDF.png"),      tr("Add PDF"), this);
	actionReadPDF  = new QAction(QIcon(":/Images/Read.png"),     tr("Read PDF"), this);
	actionPrintMe->setCheckable(true);
	actionBookmark->setCheckable(true);

	connect(actionAddQuote, SIGNAL(triggered()),     this, SIGNAL(addQuote()));
	connect(actionPrintMe,  SIGNAL(triggered(bool)), this, SIGNAL(printMe(bool)));
	connect(actionBookmark, SIGNAL(triggered(bool)), this, SIGNAL(bookmark(bool)));
	connect(actionAddPDF,   SIGNAL(triggered()),     this, SIGNAL(addPDF()));
    connect(actionReadPDF,  SIGNAL(triggered()),     this, SIGNAL(readPDF()));

    connect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(onSectionClicked(int,Qt::SortOrder)));
}

void PaperTableView::sortByColumn(int column, Qt::SortOrder order)
{
    _sortedColumn = column;
    _sortedOrder = order;
    AutoSizeTableView::sortByColumn(column, order);
}

void PaperTableView::reSort() {
    AutoSizeTableView::sortByColumn(_sortedColumn, _sortedOrder);
}

void PaperTableView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!indexAt(event->pos()).isValid())
		return;

	int paperID = model()->data(model()->index(rowAt(event->pos().y()), PAPER_ID)).toInt();
	actionPrintMe ->setChecked(isPaperForPrint(paperID));
	actionBookmark->setChecked(!isPaperRead(paperID));
    actionReadPDF ->setEnabled(pdfAttached(paperID));

	QMenu menu(this);
	menu.addAction(actionPrintMe);
	menu.addAction(actionBookmark);
	menu.addAction(actionAddQuote);
	menu.addAction(actionAddPDF);
	menu.addAction(actionReadPDF);
    menu.exec(event->globalPos());
}

void PaperTableView::onSectionClicked(int logicalIndex, Qt::SortOrder order)
{
    _sortedColumn = logicalIndex;
    _sortedOrder  = order;
}
