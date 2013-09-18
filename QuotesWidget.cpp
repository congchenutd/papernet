#include "QuotesWidget.h"
#include "QuoteDlg.h"

QuotesWidget::QuotesWidget(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);

    _paperID = -1;
    ui.tableView->setModel(&_model);

    connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onQuoteDoubleClicked(QModelIndex)));
}

void QuotesWidget::setCentralPaper(int paperID)
{
    if(paperID < 0)
		return;
    _paperID = paperID;

    if(isVisible())  // lazy update, avoiding uncessary computation
        update();
}

void QuotesWidget::showEvent(QShowEvent*) {
	update();
}

void QuotesWidget::onQuoteDoubleClicked(const QModelIndex& idx)
{
	QuoteDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Quote"));
    dlg.setQuoteID(_model.data(_model.index(idx.row(), COL_ID)).toInt());
    if(dlg.exec() == QDialog::Accepted)   // dlg will save the changes
        setCentralPaper(_paperID);
}

void QuotesWidget::update()
{
    _model.setQuery(tr("select ID, Title, Quote from Quotes where ID in \
					(select Quote from PaperQuote where Paper = %1) order by Title")
                    .arg(_paperID));
	ui.tableView->hideColumn(COL_ID);
	ui.tableView->resizeColumnToContents(COL_TITLE);
}
