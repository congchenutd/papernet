#include "QuotesWidget.h"

QuotesWidget::QuotesWidget(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
	centralPaperID = -1;

	ui.tableView->setModel(&model);

	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onQuoteDoubleClicked(QModelIndex)));
}

void QuotesWidget::setCentralPaper(int paperID)
{
	if(paperID < 0 || paperID == centralPaperID)
		return;
	centralPaperID = paperID;

	if(isVisible())
		update();
}

void QuotesWidget::showEvent(QShowEvent*) {
	update();
}

void QuotesWidget::onQuoteDoubleClicked(const QModelIndex& idx) {
	emit doubleClicked(model.data(model.index(idx.row(), COL_ID)).toInt());
}

void QuotesWidget::update()
{
	model.setQuery(tr("select ID, Title, Quote from Quotes where ID in \
					(select Quote from PaperQuote where Paper = %1) order by Title")
					.arg(centralPaperID));
	ui.tableView->hideColumn(COL_ID);
	ui.tableView->resizeColumnToContents(COL_TITLE);
}
