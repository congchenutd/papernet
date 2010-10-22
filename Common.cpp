#include "Common.h"
#include "OptionDlg.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

QString dbName;
QString attachmentDir;
QString emptyDir;

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
	query.exec("PRAGMA foreign_keys = ON");
	query.exec("create table Papers( \
					ID       int primary key, \
					Title    varchar unique, \
					Authors  varchar, \
					Year     date,    \
					Journal  varchar, \
					Abstract varchar, \
					Note     varchar, \
					Proximity int, \
					Coauthor  int \
				)");

	query.exec("create table Tags( \
					ID int primary key, \
					Name varchar unique \
				)");

	query.exec("create table PaperTag( \
					Paper int references Papers(ID) on delete cascade on update cascade, \
					Tag   int references Tags  (ID) on delete cascade on update cascade, \
					primary key (Paper, Tag) \
				)");
}

int getNextID(const QString& tableName, const QString& sectionName)
{
	QSqlQuery query;
	query.exec(QObject::tr("select max(%1) from %2").arg(sectionName).arg(tableName));
	return query.next() ? query.value(0).toInt() + 1 : 0;
}

void delPaper(int paperID)
{
	// delete attached files
	if(!MySetting<UserSetting>::getInstance()->getKeepAttachments())
		if(!delAttachments(paperID))
			QMessageBox::critical(0, QObject::tr("Error"), QObject::tr("Attachments cannot be deleted!"));

	// delete db entry	
	QSqlQuery query;
	query.exec(QObject::tr("delete from Papers where ID = %1").arg(paperID));
}

void delTag(int tagID)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from Tags where ID = %1").arg(tagID));
}

void addPaperTag(int paperID, int tagID)
{
	QSqlQuery query;
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

bool addAttachment(int paperID, const QString& attachmentName, const QString& fileName)
{
	QString dir = getAttachmentDir(paperID);
    QDir::current().mkdir(dir);
    return QFile::copy(fileName, dir + "/" + attachmentName);
}

bool delAttachment(int paperID, const QString& attachmentName)
{
	if(!MySetting<UserSetting>::getInstance()->getKeepAttachments())
	{
		bool result = QFile::remove(getFilePath(paperID, attachmentName));
		QDir(attachmentDir).rmdir(getValidTitle(paperID));    // will fail if not empty
		return result;
	}
	return true;
}

// delete the attachment dir
bool delAttachments(int paperID)
{
	QDir dir(getAttachmentDir(paperID));
	QFileInfoList files = dir.entryInfoList(QDir::Files |QDir::NoDotAndDotDot);
	foreach(QFileInfo info, files)
		if(!QFile::remove(info.filePath()))
			return false;
	return QDir(attachmentDir).rmdir(getValidTitle(paperID));
}

QString getPaperTitle(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("select Title from Papers where ID = %1").arg(paperID));
	return query.next() ? query.value(0).toString() : QString("Error") ;
}

QString makeValidTitle(const QString& title) {
	return QString(title).replace(QRegExp("[:|?|*]"), "-");
}

QString getAttachmentDir(int paperID) {
	return attachmentDir + getValidTitle(paperID);
}

QString getValidTitle(int paperID) {
	return makeValidTitle(getPaperTitle(paperID));
}

bool addLink(int paperID, const QString& link, const QString& u)
{
	QString linkName = link;
    if(!linkName.endsWith(".url", Qt::CaseInsensitive))
		linkName.append(".url");
	if(attachmentExists(paperID, linkName))
		return false;

	QString dir = getAttachmentDir(paperID);
	QDir(".").mkdir(dir);
    QFile file(dir + "/" + linkName);
	if(file.open(QFile::WriteOnly | QFile::Truncate)) 
	{
		QString url(u);
		if(!url.startsWith("http://", Qt::CaseInsensitive))
			url.prepend("http://");
		QTextStream out(&file);
		out << "[DEFAULT]" << "\r\n"
			<< "BASEURL="  << url << "\r\n"
			<< "[InternetShortcut]" << "\r\n"
			<< "URL=" << url;
		return true;
	}
	return false;
}

void openAttachment(int paperID, const QString& attachmentName)
{
	QString filePath = getFilePath(paperID, attachmentName);
#ifdef Q_WS_WIN
	QDesktopServices::openUrl(QUrl(filePath));
#endif

#ifdef Q_WS_MAC
    if(attachmentName.endsWith(".url", Qt::CaseInsensitive))
    {
        QFile file(filePath);
        if(file.open(QFile::ReadOnly))
        {
            QTextStream is(&file);
            is.readLine();
            QString url = is.readLine(); // second line is url
            url.remove("BASEURL=");
            QDesktopServices::openUrl(QUrl(url));
        }
    }
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
#endif
}

QString getFilePath(int paperID, const QString& attachmentName) 
{
	QString path = attachmentDir + getValidTitle(paperID) + "/" + attachmentName;
#ifdef Q_WS_WIN
	return path.replace("/", "\\");
#endif

#ifdef Q_WS_MAC
	return path;
#endif
}

bool renameAttachment(int paperID, const QString& oldName, const QString& newName) {
	return QFile::rename(getFilePath(paperID, oldName), getFilePath(paperID, newName));
}

bool attachmentExists(int paperID, const QString& name) {
	return QFile::exists(getFilePath(paperID, name));
}

bool renameTitle(const QString& oldName, const QString& newName) {
	return QDir(".").rename(attachmentDir + makeValidTitle(oldName), 
							attachmentDir + makeValidTitle(newName));
}

int getMaxProximity()
{
	QSqlQuery query;
	query.exec(QObject::tr("select max(Proximity) from Papers"));
	return query.next() ? query.value(0).toInt() : 0;
}

int getMaxCoauthor()
{
	QSqlQuery query;
	query.exec(QObject::tr("select max(Coauthor) from Papers"));
	return query.next() ? query.value(0).toInt() : 0;
}
