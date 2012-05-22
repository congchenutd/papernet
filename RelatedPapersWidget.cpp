#include "RelatedPapersWidget.h"
#include "Thesaurus.h"
#include <QSqlQuery>

RelatedPapersWidget::RelatedPapersWidget(QWidget* parent) :	QWidget(parent)
{
	ui.setupUi(this);
	centralPaperID = -1;

	thesaurus = new BigHugeThesaurus(this);
	connect(thesaurus, SIGNAL(response(QStringList)), this, SLOT(onThesaurus(QStringList)));

	model.setColumnCount(3);
	model.setHeaderData(COL_ID,        Qt::Horizontal, tr("ID"));
	model.setHeaderData(COL_TITLE,     Qt::Horizontal, tr("Title"));
	model.setHeaderData(COL_PROXIMITY, Qt::Horizontal, tr("Proximity"));
	ui.listView->setModel(&model);
	ui.listView->setModelColumn(COL_TITLE);

	connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onPaperDoubleClicked(QModelIndex)));
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

    // gather related tags: tags this phrase has (direct), and their proximate tags (from tagThesaurus)
    QSqlQuery query;    // query direct tags' names
    query.exec(tr("select Tags.Name from Tags, PaperTag \
                   where PaperTag.Paper = %1 and PaperTag.Tag = Tags.ID").arg(centralPaperID));
    while(query.next())
        thesaurus->request(query.value(0).toString());   // query proximate tags with this direct tag

    // calculate proximity by direct tags:
    // 1. find in PaperTag all direct tags
    // 2. count the # of all other papers that have these tags
    // 3. associate the # with the papers
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

    sort();
}

void RelatedPapersWidget::showEvent(QShowEvent*) {
    update();
}

void RelatedPapersWidget::onThesaurus(const QStringList& relatedTags)
{
	if(relatedTags.isEmpty())
		return;

	QSqlDatabase::database().transaction();

	// get the IDs of relatedTags
	QSqlQuery query;
	QStringList tagIDs;
	foreach(QString tagName, relatedTags)
	{
		query.prepare("select ID from Tags where Name = :tagName ");
		query.bindValue(":tagName", tagName);
		query.exec();
		while(query.next())
			tagIDs << query.value(0).toString();
	}

	// calculate proximity by proximate tags:
	// 1. count the # of all other papers that have these tags
	// 2. associate the # with the papers
	query.exec(tr("select Papers.ID, Paper.Title, count(Paper) Proximity from Papers, PaperTag \
				   where Tag in (%1) \
						 and PaperTag.Paper != %2 and Papers.ID = PaperTag.Paper \
                   group by PaperTag.Paper order by Proximity desc").arg(tagIDs.join(",")).arg(centralPaperID));

	QSqlDatabase::database().commit();

	// update proximity
	while(query.next())
	{
		int lastRow = model.rowCount();
		model.insertRow(lastRow);
		model.setData(model.index(lastRow, COL_ID),        query.value(0).toInt());
		model.setData(model.index(lastRow, COL_TITLE),     query.value(1).toString());
		model.setData(model.index(lastRow, COL_PROXIMITY), query.value(2).toInt());
	}

	sort();
}

void RelatedPapersWidget::onPaperDoubleClicked(const QModelIndex& idx) {
	emit doubleClicked(model.data(model.index(idx.row(), COL_ID)).toInt());
}

void RelatedPapersWidget::sort()
{
	model.sort(COL_TITLE);
    model.sort(COL_PROXIMITY, Qt::DescendingOrder);
}
