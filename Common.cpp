#include "Common.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QFile>

bool openDB(const QString& name)
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

void createTables()
{
	QSqlQuery query;
	query.exec("create table Papers( \
			   ID       int primary key, \
			   Title    varchar unique, \
			   Authors  varchar, \
			   Journal  varchar, \
			   Abstract varchar, \
			   Note     varchar, \
			   PDF      varchar)");

	query.exec("create table Tags(ID int primary key, Name varchar unique)");
	query.exec("create table PaperTag( \
				Paper int, \
				Tag int, \
				primary key (Paper, Tag))");
}

int getNextID(const QString& tableName, const QString& sectionName)
{
	QSqlQuery query;
	query.exec(QObject::tr("select max(%1) from %2").arg(sectionName).arg(tableName));
	return query.next() ? query.value(0).toInt() + 1 : 0;
}

void delPaper(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from Papers where ID = %1").arg(paperID));
	delPaperTagByPaper(paperID);
}

void delTag(int tagID)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from Tags where ID = %1").arg(tagID));
	delPaperTagByTag(tagID);
}

void delPDF(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("select PDF from Papers where ID = %1").arg(paperID));
	if(query.next())
		QFile::remove(query.value(0).toString());
}

void addPaperTag(int paperID, int tagID)
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

void delPaperTag(int paperID, int tagID)
{
	QSqlQuery query;
	bool result = query.exec(QObject::tr("delete from PaperTag where Paper=%1 and Tag=%2")
		.arg(paperID).arg(tagID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}

void delPaperTagByPaper(int paperID)
{
	QSqlQuery query;
	bool result = query.exec(QObject::tr("delete from PaperTag where Paper=%1").arg(paperID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}

void delPaperTagByTag(int tagID)
{
	QSqlQuery query;
	bool result = query.exec(QObject::tr("delete from PaperTag where Tag=%1").arg(tagID));
	if(!result)
		QMessageBox::critical(0, "error", query.lastError().text());
}