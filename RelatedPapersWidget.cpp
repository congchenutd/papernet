#include "RelatedPapersWidget.h"
#include "OptionDlg.h"
#include <QSqlQuery>

RelatedPapersWidget::RelatedPapersWidget(QWidget* parent) :	QWidget(parent)
{
	ui.setupUi(this);
    ui.tableView->init("RelatedPapers", UserSetting::getInstance());

	_centralPaperID = -1;

    _model.setColumnCount(5);
	_model.setHeaderData(COL_ID,        Qt::Horizontal, tr("ID"));
	_model.setHeaderData(COL_TITLE,     Qt::Horizontal, tr("Title"));
	_model.setHeaderData(COL_PROXIMITY, Qt::Horizontal, tr("Proximity"));
    _model.setHeaderData(COL_AUTHORS,   Qt::Horizontal, tr("Authors"));
    _model.setHeaderData(COL_YEAR,      Qt::Horizontal, tr("Year"));

    ui.tableView->setModel(&_model);
    ui.tableView->hideColumn(COL_ID);
    ui.tableView->hideColumn(COL_PROXIMITY);

    connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRelatedDoubleClicked(QModelIndex)));
}

void RelatedPapersWidget::setCentralPaper(int paperID)
{
    if(paperID < 0 || paperID == _centralPaperID)
        return;
    _centralPaperID = paperID;

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
    query.exec(tr("select ID from Tags where Name=\'%1\'").arg("ReadMe"));
    int readMe = query.next() ? query.value(0).toInt() : -1;  // ignore ReadMe
    query.exec(tr("select Papers.ID, Papers.Title, count(Paper) Proximity, Papers.Authors, Papers.Year \
                   from Papers, PaperTag \
                   where Tag in (select Tag from PaperTag where Paper = %1) \
                         and PaperTag.Paper != %1 and Papers.ID = PaperTag.Paper \
                         and Tag != %2\
                   group by PaperTag.Paper \
                   order by Proximity desc")
            .arg(_centralPaperID)
            .arg(readMe));

    QSqlDatabase::database().commit();

    // save proximity
    _model.removeRows(0, _model.rowCount());
    while(query.next())
    {
        int lastRow = _model.rowCount();
        _model.insertRow(lastRow);
        _model.setData(_model.index(lastRow, COL_ID),        query.value(COL_ID)       .toInt());
        _model.setData(_model.index(lastRow, COL_TITLE),     query.value(COL_TITLE)    .toString());
        _model.setData(_model.index(lastRow, COL_PROXIMITY), query.value(COL_PROXIMITY).toInt());
        _model.setData(_model.index(lastRow, COL_AUTHORS),   query.value(COL_AUTHORS)  .toString());
        _model.setData(_model.index(lastRow, COL_YEAR),      query.value(COL_YEAR)     .toInt());
    }

    _model.sort(COL_TITLE);
    _model.sort(COL_PROXIMITY, Qt::DescendingOrder);
}

void RelatedPapersWidget::showEvent(QShowEvent*) {
    update();
}

void RelatedPapersWidget::onRelatedDoubleClicked(const QModelIndex& idx) {
    emit doubleClicked(_model.data(_model.index(idx.row(), COL_ID)).toInt());  // send paper id
}
