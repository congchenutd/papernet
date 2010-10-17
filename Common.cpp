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
	query.exec("create table Papers( \
					ID       int primary key, \
					Title    varchar unique, \
					Authors  varchar, \
					Year     date,    \
					Journal  varchar, \
					Abstract varchar, \
					Note     varchar)");

	query.exec("create table Tags(ID int primary key, Name varchar unique)");

	query.exec("create table PaperTag( \
					Paper int, \
					Tag int, \
					primary key (Paper, Tag))");

	query.exec("create table Attachments( \
					Paper      int, \
					Attachment varchar, \
					primary key (Paper, Attachment))");
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
		delAttachments(paperID);

	// delete paper-tag entries
	QSqlQuery query;
	delPaperTagByPaper(paperID);

	// delete attachments entry
	query.exec(QObject::tr("delete from Attachments where Paper = %1").arg(paperID));

	// delete db entry
	query.exec(QObject::tr("delete from Papers where ID = %1").arg(paperID));
}

void delTag(int tagID)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from Tags where ID = %1").arg(tagID));
	
	delPaperTagByTag(tagID);  // paper-tag entries
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

bool addAttachment(int paperID, const QString& attachmentName, const QString& fileName)
{
	if(attachmentExists(paperID, attachmentName))
		return false;

	QString dir = getAttachmentDir(paperID);
	QDir(".").mkdir(dir);
	QFile::copy(fileName, dir + "\\" + attachmentName);

	QSqlQuery query;
	return query.exec(QObject::tr("insert into Attachments values (%1, \'%2\')")
					.arg(paperID).arg(attachmentName));
}

void delAttachment(int paperID, const QString& attachmentName)
{
	// delete attached file
	if(!MySetting<UserSetting>::getInstance()->getKeepAttachments())
	{
		QFile::remove(getFilePath(paperID, attachmentName));
		QDir(attachmentDir).rmdir(getValidTitle(paperID));    // will fail if not empty
	}

	// delete entry
	QSqlQuery query;
	query.exec(QObject::tr("delete from Attachments where \
			Paper = %1 and Attachment = \'%2\'").arg(paperID).arg(attachmentName));
}

// delete the attachment dir
void delAttachments(int paperID)
{
	QDir dir(getAttachmentDir(paperID));
	QFileInfoList files = dir.entryInfoList(QDir::Files |QDir::NoDotAndDotDot);
	foreach(QFileInfo info, files)
		QFile::remove(info.filePath());
	QDir(attachmentDir).rmdir(getValidTitle(paperID));
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
	QString linkName = link + ".url";
	if(attachmentExists(paperID, linkName))
		return false;

	QString dir = getAttachmentDir(paperID);
	QDir(".").mkdir(dir);
	QFile file(dir + "\\" + linkName);
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
	}

	QSqlQuery query;
	return query.exec(QObject::tr("insert into Attachments values (%1, \'%2\')")
		.arg(paperID).arg(linkName));
}

void openAttachment(int paperID, const QString& attachmentName)
{
	QString filePath = getFilePath(paperID, attachmentName);
	QDesktopServices::openUrl(QUrl(filePath));
}

QString getFilePath(int paperID, const QString& attachmentName) {
	return attachmentDir + getValidTitle(paperID) + "\\" + attachmentName;
}

bool renameAttachment(int paperID, const QString& oldName, const QString& newName)
{
	if(attachmentExists(paperID, newName))
		return false;

	QSqlQuery query;
	QFile::rename(getFilePath(paperID, oldName), getFilePath(paperID, newName));
	query.exec(QObject::tr("update Attachments set Attachment = \'%1\'  \
			where Paper = %2 and Attachment = \'%3\'")
			.arg(newName).arg(paperID).arg(oldName));
	return true;
}

bool attachmentExists(int paperID, const QString& name)
{
	QSqlQuery query;
	query.exec(QObject::tr("select * from Attachments where \
						   Paper = %1 and Attachment = \'%2\'").arg(paperID).arg(name));
	return query.next();
}

bool renameTitle(const QString& oldName, const QString& newName) {
	return QDir(".").rename(attachmentDir + makeValidTitle(oldName), 
							attachmentDir + makeValidTitle(newName));
}