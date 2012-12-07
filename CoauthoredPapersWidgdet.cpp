#include "CoauthoredPapersWidgdet.h"
#include "Common.h"
#include "EnglishName.h"
#include "OptionDlg.h"
#include <QSqlQuery>

CoauthoredPapersWidgdet::CoauthoredPapersWidgdet(QWidget *parent) :
	QWidget(parent)
{
	ui.setupUi(this);
    ui.tableView->init("CoauthoredPapers", UserSetting::getInstance());
	_centralPaperID = -1;

    _model.setColumnCount(5);
	_model.setHeaderData(COL_ID,         Qt::Horizontal, tr("ID"));
	_model.setHeaderData(COL_TITLE,      Qt::Horizontal, tr("Title"));
	_model.setHeaderData(COL_COAUTHERED, Qt::Horizontal, tr("Coauthered"));
    _model.setHeaderData(COL_AUTHORS,    Qt::Horizontal, tr("Authors"));
    _model.setHeaderData(COL_YEAR,       Qt::Horizontal, tr("Year"));

    ui.tableView->setModel(&_model);
    ui.tableView->hideColumn(COL_ID);
    ui.tableView->hideColumn(COL_COAUTHERED);

    connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onPaperDoubleClicked(QModelIndex)));
}

void CoauthoredPapersWidgdet::setCentralPaper(int paperID)
{    
    if(paperID < 0 || paperID == _centralPaperID)
        return;
    _centralPaperID = paperID;

    if(isVisible())
        update();   // lazy update, avoiding uncessary computation
}

void CoauthoredPapersWidgdet::saveSectionSizes() {
    ui.tableView->saveSectionSizes();
}

void CoauthoredPapersWidgdet::showEvent(QShowEvent*) {
    update();
}

void CoauthoredPapersWidgdet::update()
{
    QSqlDatabase::database().transaction();
    _model.removeRows(0, _model.rowCount());

	// find authors from central paper
    QSqlQuery query;
    query.exec(tr("select Authors from Papers where ID = %1").arg(_centralPaperID));
    if(query.next())
    {
		QStringList authors = splitLine(query.value(0).toString(), ";");
        foreach(const QString& author, authors)
        {
			// for each other paper
            query.exec(tr("select ID, Title, Authors, Year from Papers where ID != %1")
					   .arg(_centralPaperID));
            while(query.next())
            {
				// see if it has common names with the central paper
				QStringList names = splitLine(query.value(2).toString(), ";");
                foreach(const QString& name, names)          // for each common author
                    if(EnglishName::compare(name, author))   // update the coauthor #
                        updateCoauthorRecord(query.value(0).toInt(),
                                             query.value(1).toString(),
                                             query.value(2).toString(),
                                             query.value(3).toInt());
            }
        }
        _model.sort(COL_TITLE);                            // sort by title 2ndly
        _model.sort(COL_COAUTHERED, Qt::DescendingOrder);  // sort by coauthered 1st
    }

    QSqlDatabase::database().commit();
}

void CoauthoredPapersWidgdet::updateCoauthorRecord(int paperID, const QString& paperTitle,
                                                   const QString& authors, int year)
{
    if(paperID == _centralPaperID)
		return;

    // try to find existing paper with the id
	QModelIndexList idxes = _model.match(_model.index(0, COL_ID),
                                        Qt::DisplayRole, paperID, 1, Qt::MatchExactly);
	if(idxes.isEmpty())    // new record
	{
		int lastRow = _model.rowCount();
		_model.insertRow(lastRow);
        _model.setData(_model.index(lastRow, COL_ID),         paperID);
        _model.setData(_model.index(lastRow, COL_TITLE),      paperTitle);
		_model.setData(_model.index(lastRow, COL_COAUTHERED), 1);
        _model.setData(_model.index(lastRow, COL_AUTHORS),    authors);
        _model.setData(_model.index(lastRow, COL_YEAR),       year);
	}
    else                   // update existing record's coauthor #
	{
		int row = idxes.front().row();
		int coauthored = _model.data(_model.index(row, COL_COAUTHERED)).toInt();
		_model.setData(_model.index(row, COL_COAUTHERED), coauthored + 1);
	}
}

void CoauthoredPapersWidgdet::onPaperDoubleClicked(const QModelIndex& idx) {
	emit doubleClicked(_model.data(_model.index(idx.row(), COL_ID)).toInt());
}
