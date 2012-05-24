#include "RelatedPapersWidget.h"
#include <QSqlQuery>

RelatedPapersWidget::RelatedPapersWidget(QWidget* parent) :	QWidget(parent)
{
	ui.setupUi(this);
	centralPaperID = -1;

	model.setColumnCount(3);
	model.setHeaderData(COL_ID,        Qt::Horizontal, tr("ID"));
	model.setHeaderData(COL_TITLE,     Qt::Horizontal, tr("Title"));
	model.setHeaderData(COL_PROXIMITY, Qt::Horizontal, tr("Proximity"));
	ui.listView->setModel(&model);
	ui.listView->setModelColumn(COL_TITLE);

    connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRelatedDoubleClicked(QModelIndex)));
}

void RelatedPapersWidget::setCentralPaper(int paperID)
{
    if(paperID < 0 || paperID == centralPaperID)
        return;
    centralPaperID = paperID;

    if(isVisible())
        update();
}

void RelatedPapersWidget::update()
{
    QSqlDatabase::database().transaction();

    // calculate proximity by direct tags:
    // 1. find in PaperTag all direct tags
    // 2. count the # of all other papers that have these tags
    // 3. associate the # with the papers
    QSqlQuery query;    // query direct tags' names
    query.exec(tr("select Papers.ID, Papers.Title, count(Paper) Proximity from Papers, PaperTag \
                   where Tag in (select Tag from PaperTag where Paper = %1) \
                         and PaperTag.Paper != %1 and Papers.ID = PaperTag.Paper \
                   group by PaperTag.Paper order by Proximity desc").arg(centralPaperID));

    QSqlDatabase::database().commit();

    // save proximity
    model.removeRows(0, model.rowCount());
    while(query.next())
    {
        int lastRow = model.rowCount();
        model.insertRow(lastRow);
        model.setData(model.index(lastRow, COL_ID),        query.value(0).toInt());
        model.setData(model.index(lastRow, COL_TITLE),     query.value(1).toString());
        model.setData(model.index(lastRow, COL_PROXIMITY), query.value(2).toInt());
    }

    model.sort(COL_TITLE);
    model.sort(COL_PROXIMITY, Qt::DescendingOrder);
}

void RelatedPapersWidget::showEvent(QShowEvent*) {
    update();
}

void RelatedPapersWidget::onRelatedDoubleClicked(const QModelIndex& idx) {
    emit doubleClicked(model.data(model.index(idx.row(), COL_ID)).toInt());
}
