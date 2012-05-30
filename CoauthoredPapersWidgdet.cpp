#include "CoauthoredPapersWidgdet.h"
#include "Common.h"
#include "EnglishName.h"
#include <QSqlQuery>

CoauthoredPapersWidgdet::CoauthoredPapersWidgdet(QWidget *parent) :
	QWidget(parent)
{
	ui.setupUi(this);
	centralPaperID = -1;

	model.setColumnCount(3);
	model.setHeaderData(COL_ID,         Qt::Horizontal, tr("ID"));
	model.setHeaderData(COL_TITLE,      Qt::Horizontal, tr("Title"));
	model.setHeaderData(COL_COAUTHERED, Qt::Horizontal, tr("Coauthered"));
	ui.listView->setModel(&model);
	ui.listView->setModelColumn(COL_TITLE);

    connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onPaperDoubleClicked(QModelIndex)));
}

void CoauthoredPapersWidgdet::setCentralPaper(int paperID)
{    
    if(paperID < 0 || paperID == centralPaperID)
        return;
    centralPaperID = paperID;

    if(isVisible())
        update();
}

void CoauthoredPapersWidgdet::showEvent(QShowEvent*) {
    update();
}

void CoauthoredPapersWidgdet::update()
{
    QSqlDatabase::database().transaction();
    model.removeRows(0, model.rowCount());

	// find authors from central paper
    QSqlQuery query;
    query.exec(tr("select Authors from Papers where ID = %1").arg(centralPaperID));
    if(query.next())
    {
		QStringList authors = splitLine(query.value(0).toString(), ";");
        foreach(const QString& author, authors)
        {
			// for each other paper
			query.exec(tr("select ID, Title, Authors from Papers where ID != %1")
					   .arg(centralPaperID));
            while(query.next())
            {
				// see if it has common names with the central paper
				QStringList names = splitLine(query.value(2).toString(), ";");
                foreach(const QString& name, names)
                    if(EnglishName::compare(name, author))
                        updateRecord(query.value(0).toInt(), query.value(1).toString());
            }
        }
        model.sort(COL_TITLE);                            // sort by title 2ndly
        model.sort(COL_COAUTHERED, Qt::DescendingOrder);  // sort by coauthered 1st
    }

    QSqlDatabase::database().commit();
}

void CoauthoredPapersWidgdet::updateRecord(int id, const QString& title)
{
	if(id == centralPaperID)
		return;
	QModelIndexList idxes = model.match(model.index(0, COL_ID),
										Qt::DisplayRole, id, 1, Qt::MatchExactly);
	if(idxes.isEmpty())
	{
		int lastRow = model.rowCount();
		model.insertRow(lastRow);
		model.setData(model.index(lastRow, COL_ID),         id);
		model.setData(model.index(lastRow, COL_TITLE),      title);
		model.setData(model.index(lastRow, COL_COAUTHERED), 1);
	}
	else
	{
		int row = idxes.front().row();
		model.setData(model.index(row, COL_ID),    id);
		model.setData(model.index(row, COL_TITLE), title);
		int coauthored = model.data(model.index(row, COL_COAUTHERED)).toInt();
		model.setData(model.index(row, COL_COAUTHERED), coauthored + 1);
	}
}

void CoauthoredPapersWidgdet::onPaperDoubleClicked(const QModelIndex& idx) {
	emit doubleClicked(model.data(model.index(idx.row(), COL_ID)).toInt());
}
