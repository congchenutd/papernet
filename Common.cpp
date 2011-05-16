#include "Common.h"
#include "OptionDlg.h"
#include "Pdf2Text.h"
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
#include <QProcess>
#include <QProcessEnvironment>

#ifdef Q_WS_WIN
#include "windows.h"
#endif

QString userName;
QString dbName;
QString attachmentDir;
QString emptyDir;
QString pdfDir;

bool openDB(const QString& name)
{
	QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
	database.setDatabaseName(name);
	if(!database.open())
	{
		QMessageBox::critical(0, "Error", QObject::tr("Can not open database %1").arg(name));
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
					Name varchar unique, \
					Size int \
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
	query.exec("create table Dictionary( \
					ID int primary key, \
					Phrase      varchar, \
					Explanation varchar \
			   )");
	query.exec("create table DictionaryTags( \
					ID int primary key, \
					Name varchar unique, \
					Size int \
			  )");
	query.exec("create table PhraseTag( \
					Phrase int references Papers(ID) on delete cascade on update cascade, \
					Tag    int references Tags  (ID) on delete cascade on update cascade, \
					primary key (Paper, Tag) \
			   )");
}

int getNextID(const QString& tableName, const QString& sectionName)
{
	if(tableName.isEmpty() || sectionName.isEmpty())
		return 0;

	QSqlQuery query;
	query.exec(QObject::tr("select max(%1) from %2").arg(sectionName).arg(tableName));
	return query.next() ? query.value(0).toInt() + 1 : 0;
}

void delPaper(int paperID)
{
	if(paperID < 0)
		return;

	// delete attached files
	if(!MySetting<UserSetting>::getInstance()->getKeepAttachments())
		delAttachments(paperID);

	// delete db entries
	QSqlQuery query;
	query.exec(QObject::tr("delete from Papers where ID = %1").arg(paperID));
	query.exec(QObject::tr("delete from PaperTag where Paper = %1").arg(paperID));
	query.exec(QObject::tr("delete from PaperSnippet where Paper = %1").arg(paperID));
}

int getTagID(const QString& tagName)
{
	if(tagName.isEmpty())
		return -1;
	QSqlQuery query;
	query.exec(QObject::tr("select ID from Tags where Name = \"%1\"").arg(tagName));
	return query.next() ? query.value(0).toInt() : -1;
}

void delTag(const QString& tagName)
{
	int tagID = getTagID(tagName);
	if(tagID < 0)
		return;
	QSqlQuery query;
	query.exec(QObject::tr("delete from Tags where ID = %1").arg(tagID));
	query.exec(QObject::tr("delete from PaperTag where Tag=%1").arg(tagID));
}

void addPaperTag(int paperID, int tagID)
{
	if(paperID < 0 || tagID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("insert into PaperTag values(%1, %2)")
												.arg(paperID).arg(tagID));
	updateTagSize(tagID);
}

void delPaperTag(int paperID, int tagID)
{
	if(paperID < 0 || tagID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("delete from PaperTag where Paper=%1 and Tag=%2")
												.arg(paperID).arg(tagID));
	updateTagSize(tagID);
}

bool addAttachment(int paperID, const QString& attachmentName, const QString& filePath)
{
	if(paperID < 0 || attachmentName.isEmpty() || filePath.isEmpty())
		return false;

	QString dir = getAttachmentDir(paperID);
	QDir::current().mkdir(dir);  // make attachment dir for this paper
	QString targetFilePath;
	if(attachmentName.compare("Paper.pdf", Qt::CaseInsensitive) == 0)   // pdf
	{
		targetFilePath = getPDFPath(paperID);

		// create shortcut
		QFile link(dir + "/Paper.pdf");
		if(!link.open(QFile::WriteOnly | QFile::Truncate))
			return false;

		// create full text
		QString fullTextFilePath = dir + "/" + "fulltext.txt";
		Pdf2Text(filePath.toAscii(), fullTextFilePath.toAscii());
		hideFile(fullTextFilePath);
	}
	else {
		targetFilePath = dir + "/" + attachmentName;
	}

	return QFile::copy(filePath, targetFilePath);
}

// del one attachment of the paper
void delAttachment(int paperID, const QString& attachmentName)
{
	if(paperID < 0 || attachmentName.isEmpty())
		return;

	QFile::remove(getAttachmentPath(paperID, attachmentName));            // attachment itself
	if(attachmentName.compare("Paper.pdf", Qt::CaseInsensitive) == 0)     // for pdf
	{
		QFile::remove(getPDFPath(paperID));                               // remove [title].pdf
		QFile::remove(getAttachmentDir(paperID) + "/" + "fulltext.txt");  // remove full text file
	}
	QDir(attachmentDir).rmdir(getValidTitle(paperID));    // del attachment dir, invalid if not empty
}

// delete the attachment dir and its contents
void delAttachments(int paperID)
{
	if(paperID < 0)
		return;

	QFileInfoList files = QDir(getAttachmentDir(paperID)).entryInfoList(QDir::Files | QDir::Hidden);
	foreach(QFileInfo info, files)                       // del all files in the attachment dir
		QFile::remove(info.filePath());
	QDir(attachmentDir).rmdir(getValidTitle(paperID));   // del attachment dir
	QFile::remove(getPDFPath(paperID));                  // del [title].pdf
}

QString getPaperTitle(int paperID)
{
	if(paperID < 0)
		return QString("Error");

	QSqlQuery query;
	query.exec(QObject::tr("select Title from Papers where ID = %1").arg(paperID));
	return query.next() ? query.value(0).toString() : QString("Error");
}

// remove illegal chars
QString makeValidTitle(const QString& title)
{
	if(title.isEmpty())
		return title;

	QString result = title;
	result.replace(QRegExp("[:|?|*]"), "-");
	result.remove('\"');
	result.remove('\'');
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
	if(paperID < 0 || link.isEmpty() || u.isEmpty())
		return false;

	QString linkName = link;
	if(!linkName.endsWith(".url", Qt::CaseInsensitive))
		linkName.append(".url");

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

void openUrl(const QString& url)
{
	if(url.isEmpty())
		return;

#ifdef Q_WS_WIN
	QDesktopServices::openUrl(QUrl(url));
#endif
#ifdef Q_WS_MAC
	QDesktopServices::openUrl(QUrl::fromLocalFile(url));  // no idea why different
#endif
}

void openAttachment(int paperID, const QString& attachmentName)
{
	if(paperID < 0 || attachmentName.isEmpty())
		return;

	QString filePath = getAttachmentPath(paperID, attachmentName);
	QString url = filePath;

	// Paper.pdf is not a real file, convert it to real pdf file
	if(attachmentName.compare("Paper.pdf", Qt::CaseInsensitive) == 0)
		url = convertSlashes(getPDFPath(paperID));

#ifdef Q_WS_MAC
	if(attachmentName.endsWith(".url", Qt::CaseInsensitive))  // mac opens url differently
	{
		QFile file(filePath);
		if(file.open(QFile::ReadOnly))
		{
			QTextStream is(&file);
			is.readLine();
			url = is.readLine();      // second line is url
			url.remove("BASEURL=");
		}
	}
#endif

	openUrl(url);
}

QString getAttachmentPath(int paperID, const QString& attachmentName) {
	return convertSlashes(attachmentDir + getValidTitle(paperID) + "/" + attachmentName);
}

QString convertSlashes(const QString& link)
{
	QString result = link;

#ifdef Q_WS_WIN
	return result.replace("/", "\\");
#endif

	return result;
}

bool renameAttachment(int paperID, const QString& oldName, const QString& newName) {
	return QFile::rename(getAttachmentPath(paperID, oldName), getAttachmentPath(paperID, newName));
}

bool renameTitle(const QString& oldName, const QString& newName)
{
	if(oldName == newName)
		return true;
	return QFile::rename(pdfDir + "/" + oldName + ".pdf",               // rename [title].pdf
						 pdfDir + "/" + newName + ".pdf") &&
			QDir(".").rename(attachmentDir + makeValidTitle(oldName),
							 attachmentDir + makeValidTitle(newName));  // rename attachment dir
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

void updateQuote(int id, const QString& title, const QString& content)
{
	if(id < 0 || title.isEmpty() || content.isEmpty())
		return;

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

void addPaperQuote(int paperID, int snippetID)
{
	if(paperID < 0 || snippetID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("insert into PaperSnippet values (%1, %2)")
										.arg(paperID).arg(snippetID));
}

// add a paper without detail info
void addSimplePaper(int id, const QString& title)
{
	if(id < 0 || title.isEmpty())
		return;

	QSqlQuery query;
	query.exec(QObject::tr("insert into Papers(ID, Title) values (%1, \"%2\")")
													.arg(id).arg(title));
}

void delQuote(int id)
{
	if(id < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("delete from Snippets where ID = %1").arg(id));
	query.exec(QObject::tr("delete from PaperSnippet where Snippet = %1").arg(id));
}

bool isTagged(int paperID)
{
	if(paperID < 0)
		return false;

	QSqlQuery query;
	query.exec(QObject::tr("select * from PaperTag where Paper = %1").arg(paperID));
	return query.next();
}

AttachmentStatus isAttached(int paperID)
{
	if(paperID < 0)
		return ATTACH_NONE;
	QFileInfoList infos = QDir(getAttachmentDir(paperID)).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	if(infos.isEmpty())
		return ATTACH_NONE;

	QSet<QString> suffixes;
	foreach(QFileInfo info, infos)
		suffixes << info.suffix().toLower();

	QSet<QString> citations;
	citations << "enw" << "ris";

	if(!suffixes.intersect(citations).isEmpty())      // has endnote files
	{
		QSet<QString> suffixes2 = suffixes;
		if(!suffixes2.subtract(citations).isEmpty())  // also has other files
			return ATTACH_ALL;
		return ATTACH_ENDNOTE;
	}
	return ATTACH_PAPER;
}

void setPaperRead(int paperID)
{
	if(paperID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("update Papers set Read = \'true\' where ID = %1").arg(paperID));
}

void updateAttached(int paperID)  // update Attached section of Papers table
{
	if(paperID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("update Papers set Attached = %1 where ID = %2")
								.arg(isAttached(paperID)).arg(paperID));
}

int getQuoteID(const QString& title)
{
	QSqlQuery query;
	query.exec(QObject::tr("select ID from Snippets where Title = \"%1\"").arg(title));
	return query.next() ? query.value(0).toInt() : -1;
}

bool fullTextSearch(int paperID, const QString& target)
{
	if(paperID < 0 || target.isEmpty())
		return false;

	QString fullTextFilePath = getAttachmentDir(paperID) + "/fulltext.txt";
	QFile file(fullTextFilePath);
	if(file.open(QFile::ReadOnly))
		if(file.readAll().indexOf(target) > -1)
			return true;
	return false;
}

void makeFullTextFiles()
{
	QSqlQuery query;
	query.exec("select ID from Papers");
	while(query.next())
	{
		int id = query.value(0).toInt();
		QString dir = getAttachmentDir(id);
		QString pdf = getAttachmentPath(id, "Paper.pdf");
		if(QFile::exists(pdf))
		{
			QString fullText = dir + "/" + "fulltext.txt";
			Pdf2Text(pdf.toAscii(), fullText.toAscii());
			hideFile(fullText);
		}
	}
}

void hideFile(const QString& filePath)
{
#ifdef Q_WS_WIN
	SetFileAttributesA(filePath.toAscii(), FILE_ATTRIBUTE_HIDDEN);
#endif

#ifdef Q_WS_MAC   // don't know how to do it in Mac
#endif
}

QString getPDFPath(int paperID) {
	return pdfDir + "/" + getValidTitle(paperID) + ".pdf";
}

void updateTagSize(int tagID)
{
	if(tagID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("select count(*) from PaperTag group by Tag having Tag = %1").arg(tagID));
	int size = query.next() ? query.value(0).toInt() : 0;
	query.exec(QObject::tr("update Tags set Size = %1 where ID = %2").arg(size).arg(tagID));
}

QStringList getTags(int paperID)
{
	QStringList tags;
	QSqlQuery query;
	query.exec(QObject::tr("select Name from Tags, PaperTag where Paper = %1 and Tag = ID")
			   .arg(paperID));
	while(query.next())
		tags << query.value(0).toString();
	return tags;
}

void renameTag(const QString& oldName, const QString& newName)
{
	if(oldName.isEmpty() || newName.isEmpty())
		return;

	QSqlQuery query;
	query.exec(QObject::tr("update Tags set Name = \"%1\" where Name = \"%2\"")
			   .arg(newName).arg(oldName));
}

void addTag(int id, const QString& name)
{
	if(id < 0 || name.isEmpty())
		return;

	QSqlQuery query;
	query.exec(QObject::tr("insert into Tags values(%1, \"%2\", 0)")
			   .arg(id).arg(name));
}
