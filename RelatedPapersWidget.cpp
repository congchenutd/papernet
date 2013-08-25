#include "RelatedPapersWidget.h"
#include "OptionDlg.h"
#include <QSqlQuery>

RelatedPapersWidget::RelatedPapersWidget(QWidget* parent) :	QWidget(parent)
{
	ui.setupUi(this);
    ui.tableView->init("RelatedPapers", UserSetting::getInstance());

	centralPaperID = -1;

    model.setColumnCount(5);
	model.setHeaderData(COL_ID,        Qt::Horizontal, tr("ID"));
	model.setHeaderData(COL_TITLE,     Qt::Horizontal, tr("Title"));
	model.setHeaderData(COL_PROXIMITY, Qt::Horizontal, tr("Proximity"));
    model.setHeaderData(COL_AUTHORS,   Qt::Horizontal, tr("Authors"));
    model.setHeaderData(COL_YEAR,      Qt::Horizontal, tr("Year"));

    ui.tableView->setModel(&model);
//    ui.tableView->hideColumn(COL_ID);
    ui.tableView->hideColumn(COL_PROXIMITY);

    connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRelatedDoubleClicked(QModelIndex)));
}

void RelatedPapersWidget::setCentralPaper(int paperID)
{
    if(paperID < 0 || paperID == centralPaperID)
        return;
    centralPaperID = paperID;

    if(isVisible())
        update();
}

void RelatedPapersWidget::saveSectionSizes() {
    ui.tableView->saveSectionSizes();
}

void RelatedPapersWidget::update()
{
    QSqlDatabase::database().transaction();

    // calculate proximity by direct tags:
    // 1. find in PaperTag all direct tags
    // 2. count the # of all other papers that have these tags
    // 3. associate the # with the papers
    QSqlQuery query;    // query direct tags' names
    query.exec(tr("select Papers.ID, Papers.Title, count(Paper) Proximity, Papers.Authors, Papers.Year \
                   from Papers, PaperTag \
                   where Tag in (select Tag from PaperTag where Paper = %1) \
                         and PaperTag.Paper != %1 and Papers.ID = PaperTag.Paper \
                   group by PaperTag.Paper \
                   order by Proximity desc")
            .arg(centralPaperID));

    QSqlDatabase::database().commit();

    // save proximity
    model.removeRows(0, model.rowCount());
    while(query.next())
    {
        int lastRow = model.rowCount();
        model.insertRow(lastRow);
        model.setData(model.index(lastRow, COL_ID),        query.value(COL_ID)       .toInt());
        model.setData(model.index(lastRow, COL_TITLE),     query.value(COL_TITLE)    .toString());
        model.setData(model.index(lastRow, COL_PROXIMITY), query.value(COL_PROXIMITY).toInt());
        model.setData(model.index(lastRow, COL_AUTHORS),   query.value(COL_AUTHORS)  .toString());
        model.setData(model.index(lastRow, COL_YEAR),      query.value(COL_YEAR)     .toInt());
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
