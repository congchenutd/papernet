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
QString compileDate;

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
	query.exec("create table Quotes( \
					ID int primary key, \
					Title varchar, \
					Quote varchar  \
				)");
	query.exec("create table PaperQuote( \
					Paper int references Papers(ID) on delete cascade on update cascade, \
					Quote int references Quotes(ID) on delete cascade on update cascade, \
					primary key (Paper, Quote) \
			   )");
	query.exec("create table Dictionary( \
					ID int primary key,  \
					Phrase      varchar, \
					Explanation varchar, \
					Proximity   int      \
			   )");
	query.exec("create table DictionaryTags( \
					ID int primary key, \
					Name varchar unique, \
					Size int \
			  )");
	query.exec("create table PhraseTag( \
					Phrase int references Dictionary    (ID) on delete cascade on update cascade, \
					Tag    int references DictionaryTags(ID) on delete cascade on update cascade, \
					primary key (Phrase, Tag) \
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
	query.exec(QObject::tr("delete from PaperQuote where Paper = %1").arg(paperID));
}

int getTagID(const QString& tableName, const QString& tagName)
{
	if(tagName.isEmpty())
		return -1;
	QSqlQuery query;
	query.exec(QObject::tr("select ID from %1 where Name = \"%2\"").arg(tableName).arg(tagName));
	return query.next() ? query.value(0).toInt() : -1;
}

void delTag(const QString& tableName, const QString& tagName)
{
	int tagID = getTagID(tableName, tagName);
	if(tagID < 0)
		return;
	QSqlQuery query;
	query.exec(QObject::tr("delete from %1 where ID = %2").arg(tableName).arg(tagID));
	query.exec(QObject::tr("delete from PaperTag  where Tag=%1").arg(tagID));
	query.exec(QObject::tr("delete from PhraseTag where Tag=%1").arg(tagID));
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

void renameTitle(const QString& oldName, const QString& newName)
{
	if(oldName == newName || oldName.isEmpty() || newName.isEmpty())
		return;
	QFile::rename(pdfDir + "/" + oldName + ".pdf",              // rename [title].pdf
				  pdfDir + "/" + newName + ".pdf");
	QDir(".").rename(attachmentDir + makeValidTitle(oldName),
					 attachmentDir + makeValidTitle(newName));  // rename attachment dir
}

int getMaxProximity(const QString& tableName)
{
	QSqlQuery query;
	query.exec(QObject::tr("select max(Proximity) from %1").arg(tableName));
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
	query.exec(QObject::tr("select * from Quotes where ID = %1").arg(id));
	if(query.next())
		query.exec(QObject::tr("update Quotes set Title = \"%1\", Quote =\"%2\" \
							   where ID = %3").arg(title).arg(content).arg(id));
	else
		query.exec(QObject::tr("insert into Quotes values (%1, \"%2\", \"%3\")")
										.arg(id).arg(title).arg(content));
}

int getPaperID(const QString& title)
{
	QSqlQuery query;
	query.exec(QObject::tr("select ID from Papers where Title = \"%1\"").arg(title));
	return query.next() ? query.value(0).toInt() : -1;
}

void addPaperQuote(int paperID, int quoteID)
{
	if(paperID < 0 || quoteID < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("insert into PaperQuote values (%1, %2)")
										.arg(paperID).arg(quoteID));
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
	query.exec(QObject::tr("delete from Quotes where ID = %1").arg(id));
	query.exec(QObject::tr("delete from PaperQuote where Quote = %1").arg(id));
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

	QSet<QString> suffixesEndnote = suffixes;
	if(!suffixesEndnote.intersect(citations).isEmpty()) // has endnote files
	{
		QSet<QString> suffixesPdf = suffixes;
		if(!suffixesPdf.subtract(citations).isEmpty())  // also has other files
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
	query.exec(QObject::tr("select ID from Quotes where Title = \"%1\"").arg(title));
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

QStringList getTagsOfPaper(int paperID)
{
	QStringList tags;
	QSqlQuery query;
	query.exec(QObject::tr("select Name from Tags, PaperTag where Paper = %1 and Tag = ID")
			   .arg(paperID));
	while(query.next())
		tags << query.value(0).toString();
	return tags;
}

void renameTag(const QString& tableName, const QString& oldName, const QString& newName)
{
	if(oldName.isEmpty() || newName.isEmpty())
		return;

	QSqlQuery query;
	query.exec(QObject::tr("update %1 set Name = \"%2\" where Name = \"%3\"")
			   .arg(tableName).arg(newName).arg(oldName));
}

void delPhrase(int id)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from Dictionary where ID = %1")    .arg(id));
	query.exec(QObject::tr("delete from PhraseTag  where Phrase = %1").arg(id));
}

QStringList getTagsOfPhrase(int phraseID)
{
	QStringList tags;
	QSqlQuery query;
	query.exec(QObject::tr("select Name from DictionaryTags, PhraseTag where Phrase=%1 and Tag=ID")
															.arg(phraseID));
	while(query.next())
		tags << query.value(0).toString();
	return tags;
}

int idToRow(QAbstractItemModel* model, int idSection, int id)
{
	QModelIndexList indexes = model->match(
		model->index(0, idSection), Qt::DisplayRole, id, 1, Qt::MatchExactly | Qt::MatchWrap);
	return !indexes.isEmpty() ? indexes.at(0).row() : -1;
}

bool paperExists(const QString &title) {
	return getPaperID(title) > -1;
}

bool phraseExists(const QString& phrase)
{
	QSqlQuery query;
	query.exec(QObject::tr("select * from Dictionary where Phrase = \"%1\"").arg(phrase));
	return query.next();
}
