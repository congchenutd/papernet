#include "Common.h"
#include "OptionDlg.h"
#include "Pdf2Text.h"
#include "windows.h"
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
					Read     bool, \
					Tagged   bool, \
					Attached bool, \
					Title    varchar unique, \
					Authors  varchar, \
					Year     date,    \
					Journal  varchar, \
					Abstract varchar, \
					Note     varchar, \
					Proximity int, \
					Coauthor  int  \
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

	query.exec("create table Snippets( \
					ID int primary key, \
					Title   varchar, \
					Snippet varchar  \
				)");

	query.exec("create table PaperSnippet( \
					Paper   int references Papers  (ID) on delete cascade on update cascade, \
					Snippet int references Snippets(ID) on delete cascade on update cascade, \
					primary key (Paper, Snippet) \
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
        delAttachments(paperID);

	// delete db entry	
	QSqlQuery query;
	query.exec(QObject::tr("delete from Papers where ID = %1").arg(paperID));

	// delete papertag entry
	query.exec(QObject::tr("delete from PaperTag where Paper = %1").arg(paperID));

	// delete papersnippet entry
	query.exec(QObject::tr("delete from PaperSnippet where Paper = %1").arg(paperID));
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
	QString targetFilePath = dir + "/" + attachmentName;
    bool result = QFile::copy(fileName, targetFilePath);
	if(!result)
		return false;

	// create fulltext file for pdf
	if(targetFilePath.endsWith(".pdf", Qt::CaseInsensitive))
	{
		QString fullTextFilePath = dir + "/" + "fulltext.txt";
		Pdf2Text(targetFilePath.toAscii(), fullTextFilePath.toAscii());
		hideFile(fullTextFilePath);
	}

	return true;
}

void delAttachment(int paperID, const QString& attachmentName)
{
	if(!MySetting<UserSetting>::getInstance()->getKeepAttachments())
	{
        QFile::remove(getAttachmentPath(paperID, attachmentName));
		QDir(attachmentDir).rmdir(getValidTitle(paperID));    // will fail if not empty
	}
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

QString makeValidTitle(const QString& title) 
{
	QString result = title;
	result.replace(QRegExp("[:|?|*]"), "-");
	result.remove('\"');
	result.remove('\"');
	return result;
}

QString getAttachmentDir(int paperID) {
	return paperID > -1 ? attachmentDir + getValidTitle(paperID) : emptyDir;
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
	QString filePath = getAttachmentPath(paperID, attachmentName);

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

QString getAttachmentPath(int paperID, const QString& attachmentName) 
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
	return QFile::rename(getAttachmentPath(paperID, oldName), getAttachmentPath(paperID, newName));
}

bool attachmentExists(int paperID, const QString& name) {
	return QFile::exists(getAttachmentPath(paperID, name));
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

bool titleExists(const QString &title) {
	return getPaperID(title) > -1;
}

void updateSnippet(int id, const QString& title, const QString& content)
{
	QSqlQuery query;
	query.exec(QObject::tr("select * from Snippets where ID = %1").arg(id));
	if(query.next())
		query.exec(QObject::tr("update Snippets set Title = \"%1\", Snippet =\"%2\" \
							   where ID = %3").arg(title).arg(content).arg(id));
	else
		query.exec(QObject::tr("insert into Snippets values (%1, \"%2\", \"%3\")")
										.arg(id).arg(title).arg(content));
}

int getPaperID(const QString& title)
{
	QSqlQuery query;
	query.exec(QObject::tr("select ID from Papers where Title = \"%1\"").arg(title));
	return query.next() ? query.value(0).toInt() : -1;
}

void addPaperSnippet(int paperID, int snippetID)
{
	QSqlQuery query;
	query.exec(QObject::tr("insert into PaperSnippet values (%1, %2)")
										.arg(paperID).arg(snippetID));
}

void addPaper(int id, const QString& title)
{
	QSqlQuery query;
	query.exec(QObject::tr("insert into Papers(ID, Title) values (%1, \"%2\")")
													.arg(id).arg(title));
}

void delSnippet(int id)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from Snippets where ID = %1").arg(id));
}

QStringList getPaperList(int snippetID)
{
	QStringList result;
	QSqlQuery query;
	query.exec(QObject::tr("select Paper from PaperSnippet where Snippet = %1").arg(snippetID));
	while(query.next())
		result << getPaperTitle(query.value(0).toInt());
	return result;
}

bool isTagged(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("select * from PaperTag where Paper = %1").arg(paperID));
	return query.next();
}

AttachmentStatus isAttached(int paperID) 
{
	QFileInfoList infos = QDir(getAttachmentDir(paperID)).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	if(infos.isEmpty())
		return ATTACH_NONE;

	QSet<QString> suffixes;
	foreach(QFileInfo info, infos)
		suffixes << info.suffix().toLower();

	QSet<QString> citations;
	citations << "enw" << "ris";

	QSet<QString> suffixes2 = suffixes;
	if(!suffixes.intersect(citations).isEmpty())      // has endnote files
	{
		if(!suffixes2.subtract(citations).isEmpty())  // has more than endnote files
			return ATTACH_ALL;
		return ATTACH_ENDNOTE;
	}
	return ATTACH_PAPER;
}

void setRead(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("update Papers set Read = \'true\' where ID = %1").arg(paperID));
}

void updateTagged(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("update Papers set Tagged = %1 where ID = %2")
								.arg(isTagged(paperID)).arg(paperID));
}

void updateAttached(int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("update Papers set Attached = %1 where ID = %2")
								.arg(isAttached(paperID)).arg(paperID));
}

int getSnippetID(const QString& title)
{
	QSqlQuery query;
	query.exec(QObject::tr("select ID from Snippets where Title = \"%1\"").arg(title));
	return query.next() ? query.value(0).toInt() : -1;
}

void hideFile(const QString& filePath)
{
#ifdef Q_WS_WIN
	SetFileAttributesA(filePath.toAscii(), FILE_ATTRIBUTE_HIDDEN);
#endif

#ifdef Q_WS_MAC
#endif
}

bool fullTextSearch(int paperID, const QString& target)
{
	QString fullTextFilePath = getAttachmentDir(paperID) + "/fulltext.txt";
	QFile file(fullTextFilePath);
	if(file.open(QFile::ReadOnly))
		while(!file.atEnd())
			if(file.readLine().indexOf(target) > -1)
				return true;
	return false;
}