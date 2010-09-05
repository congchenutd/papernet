#ifndef Common_h__
#define Common_h__

#include <QtGui>
#include <QtSql>

inline bool openDB(const QString& name)
{
	QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
	database.setDatabaseName(name);
	if(!database.open())
	{
		QMessageBox::critical(0, "Error", "Can not open database");
		return false;
	}
	return true;
}

inline void createTables()
{
	QSqlQuery query;
	query.exec("create table Papers( \
			   ID int primary key, \
			   Title varchar unique, \
			   Authors varchar, \
			   Journal varchar, \
			   Abstract varchar, \
			   PDF varchar)");
	
	query.exec("create table Tags(ID int primary key, Name varchar unique)");
	query.exec("create table PaperTag(Paper int, Tag varchar)");
}

inline int getNextID(const QString& tableName, const QString& sectionName)
{
	QSqlQuery query;
	query.exec(QObject::tr("select max(%1) from %2").arg(sectionName).arg(tableName));
	return query.next() ? query.value(0).toInt() + 1 : 0;
}

inline void addPaperTag(int paperID, int tagID)
{
	QSqlQuery query;
	query.exec(QObject::tr("select * from PaperTag where Paper=%1 and Tag=\'%2\'")
		.arg(paperID).arg(tagID));
	if(query.next())
		return;

	bool result = query.exec(QObject::tr("insert into PaperTag values (%1, %2)")
								.arg(paperID).arg(tagID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}

inline void delPaperTag(int paperID, int tagID)
{
	QSqlQuery query;
	bool result = query.exec(QObject::tr("delete from PaperTag where Paper=%1 and Tag=%2")
										.arg(paperID).arg(tagID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}

inline void delPaperTagByPaper(int paperID)
{
	QSqlQuery query;
	bool result = query.exec(QObject::tr("delete from PaperTag where Paper=%1").arg(paperID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}

inline void delPaperTagByTag(int tagID)
{
	QSqlQuery query;
	bool result = query.exec(QObject::tr("delete from PaperTag where Tag=%1").arg(tagID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}

const int MAX_PAPER_COUNT = 1000;

#endif // Common_h__