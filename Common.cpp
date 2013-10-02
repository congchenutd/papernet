#include "Common.h"
#include "OptionDlg.h"
#include "EnglishName.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QProcessEnvironment>
#include <QWidget>
#include <QDesktopWidget>
#include <QSqlTableModel>

#ifdef Q_OS_WIN32
#include "windows.h"
#endif

QString dbName;
QString attachmentDir;
QString emptyDir;

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
			   ID          int primary key, \
			   Attached    bool,    \
			   Title       varchar unique, \
			   Authors     varchar, \
			   Year        date,    \
			   Modified    date,    \
			   Type        varchar, \
			   Publication varchar, \
			   Abstract    varchar, \
			   Volume      int, \
			   Issue       int, \
			   Startpage   int, \
			   Endpage     int, \
			   Publisher   varchar, \
			   Editors     varchar, \
			   Address     varchar, \
			   Url         varchar, \
			   Note        varchar  \
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
					Explanation varchar  \
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
	if(!UserSetting::getInstance()->getKeepAttachments())
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
	query.prepare(QObject::tr("select ID from %1 where Name = :tagName").arg(tableName));
	query.bindValue(":tagName", tagName);
	query.exec();
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

QString getFullTextFilePath(int paperID, const QString& attachmentName) {
	return	getAttachmentDir(paperID) +
			"/." + QFileInfo(attachmentName).baseName() + ".fulltext";
}

bool addAttachment(int paperID, const QString& attachmentName, const QString& filePath)
{
	if(paperID < 0 || attachmentName.isEmpty() || filePath.isEmpty())
		return false;

	QString dir = getAttachmentDir(paperID);
	QDir::current().mkdir(dir);  // make attachment dir for this paper

	QString targetFilePath = autoRename(dir + "/" + attachmentName);
	if(attachmentName.toLower().endsWith(".pdf"))   // create full text for pdf
		makeFullTextFile(filePath, getFullTextFilePath(paperID, targetFilePath));

	bool result = QFile::copy(filePath, targetFilePath);
	if(UserSetting::getInstance()->getMoveAttachments())   // remove the original
		QFile::remove(filePath);
	return result;
}

QString autoRename(const QString& original)
{
	QString extension = QFileInfo(original).suffix().toLower();
	QString baseName  = QFileInfo(original).baseName();

	QString result = original;
	while(QFile::exists(result))
	{
		QRegExp rxNumber("\\((\\d+)\\)");
		if(rxNumber.indexIn(baseName) > -1)   // find (number) and increase number
		{
			int number = rxNumber.cap(1).toInt();
			baseName.replace(rxNumber, "(" + QString::number(number + 1) + ")");
		}
		else {
			baseName += "(1)";                // add (1)
		}
		result = QFileInfo(original).path() + "/" + baseName + "." + extension;
	}

	return result;
}

// del one attachment of the paper
void delAttachment(int paperID, const QString& attachmentName)
{
	if(paperID < 0 || attachmentName.isEmpty())
		return;

	QFile::remove(getAttachmentPath(paperID, attachmentName));         // attachment itself
	if(attachmentName.toLower().endsWith(".pdf"))                      // for pdf
		QFile::remove(getFullTextFilePath(paperID, attachmentName));   // remove full text file
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
}

QString idToTitle(int paperID)
{
	if(paperID < 0)
		return QString("Invalid PaperID");

	QSqlQuery query;
	query.exec(QObject::tr("select Title from Papers where ID = %1").arg(paperID));
	return query.next() ? query.value(0).toString() : QString("Invalid PaperID");
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
	return paperID > -1 ? attachmentDir + getValidTitle(paperID) : QString();
}

QString getValidTitle(int paperID) {
	return makeValidTitle(idToTitle(paperID));
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

void openFile(const QString& filePath)
{
	if(filePath.isEmpty())
		return;

	if(QFileInfo(filePath).isFile())  // FIXME: somehow relative path doesn't work :(
		QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absoluteFilePath()));
	else
		QDesktopServices::openUrl(QUrl(filePath));   // Internet address
}

void openAttachment(int paperID, const QString& attachmentName)
{
	if(paperID < 0 || attachmentName.isEmpty())
		return;

	QString filePath = getAttachmentPath(paperID, attachmentName);
	QString url = filePath;

#ifdef Q_OS_MAC
	if(attachmentName.toLower().endsWith(".url"))  // mac opens url differently
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
	openFile(url);
}

QString getAttachmentPath(int paperID, const QString& attachmentName) {
	return convertSlashes(getAttachmentDir(paperID) + "/" + attachmentName);
}

QString convertSlashes(const QString& link)
{
	QString result = link;

#ifdef Q_OS_WIN32
	return result.replace("/", "\\");
#endif

	return result;
}

bool renameAttachment(int paperID, const QString& oldName, const QString& newName) {
	return QFile::rename(getAttachmentPath(paperID, oldName),
						 getAttachmentPath(paperID, newName));
}

void renameTitle(const QString& oldName, const QString& newName)
{
	if(oldName == newName || oldName.isEmpty() || newName.isEmpty())
		return;
	QDir::current().rename(attachmentDir + makeValidTitle(oldName),
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

// update the quote with the id
// insert new one if not exists
void updateQuote(int id, const QString& title, const QString& content)
{
	if(id < 0 || title.isEmpty())
		return;

	QSqlQuery query;
	query.exec(QObject::tr("select * from Quotes where ID = %1").arg(id));
	if(query.next())
		query.prepare("update Quotes set Title=:title, Quote=:content where ID=:id");
	else
		query.prepare("insert into Quotes values (:id, :title, :content)");
	query.bindValue(":id",      id);
	query.bindValue(":title",   title);
	query.bindValue(":content", content);
	query.exec();
}

int titleToID(const QString& title)
{
	QSqlQuery query;
	query.prepare("select ID from Papers where Title = :title");
	query.bindValue(":title", title);
	query.exec();
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

// add a paper without detailed info
void addSimplePaper(int id, const QString& title)
{
	if(id < 0 || title.isEmpty())
		return;

	QSqlQuery query;
	query.prepare("insert into Papers(ID, Title) values (:id, :title)");
	query.bindValue(":id",    id);
	query.bindValue(":title", title);
	query.exec();
}

void delQuote(int id)
{
	if(id < 0)
		return;

	QSqlQuery query;
	query.exec(QObject::tr("delete from Quotes where ID = %1").arg(id));
	query.exec(QObject::tr("delete from PaperQuote where Quote = %1").arg(id));
}

bool pdfAttached(int paperID)
{
	if(paperID < 0)
		return false;
	QFileInfoList infos = QDir(getAttachmentDir(paperID)).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	if(infos.isEmpty())
		return false;

	foreach(const QFileInfo& fileInfo, infos)
		if(fileInfo.suffix().toLower() == "pdf")
			return true;

	return false;
}

bool isPaperRead(int paperID) {
	return !getTagsOfPaper(paperID).contains("ReadMe");
}

bool isPaperForPrint(int paperID) {
	return getTagsOfPaper(paperID).contains("PrintMe");
}

int getQuoteID(const QString& title)
{
	QSqlQuery query;
	query.prepare("select ID from Quotes where Title = :title");
	query.bindValue(":title", title);
	query.exec();
	return query.next() ? query.value(0).toInt() : -1;
}

bool fullTextSearch(int paperID, const QString& target)
{
	if(paperID < 0 || target.isEmpty())
		return false;

	// for all fulltext files in the attachment dir
	QFileInfoList files = QDir(getAttachmentDir(paperID)).entryInfoList(QDir::Files);
	foreach(QFileInfo info, files)
		if(info.suffix().toLower() == "fulltext")
		{
			QFile file(info.filePath());
			if(file.open(QFile::ReadOnly))
				if(file.readAll().indexOf(target) > -1)
					return true;
		}
	return false;
}

void makeFullTextFile(const QString& pdfPath, const QString& fulltextPath)
{
	QString convertorPath;
#ifdef Q_OS_WIN32
	convertorPath = "./pdftotext.exe";
#endif

#ifdef Q_OS_MAC
	convertorPath = "./pdftotext";
#endif

	if(!convertorPath.isEmpty() && QFile::exists(pdfPath))
	{
		QFile::remove(fulltextPath);
		QProcess::execute(convertorPath, QStringList() << pdfPath << fulltextPath);
		hideFile(fulltextPath);
	}
}

void makeFullTextFiles()
{
	QSqlQuery query;
	query.exec("select ID from Papers");
	while(query.next())
	{
		int paperID = query.value(0).toInt();
		// for all pdf files
		QFileInfoList files = QDir(getAttachmentDir(paperID)).entryInfoList(QDir::Files | QDir::Hidden);
		foreach(QFileInfo info, files)
			if(info.suffix().toLower() == "pdf")
				makeFullTextFile(info.filePath(),
								 getFullTextFilePath(paperID, info.baseName()));
	}
}

void hideFile(const QString& filePath)
{
#ifdef Q_OS_WIN32
	// This is a windows API, not from Qt
	SetFileAttributesA(filePath.toLatin1(), FILE_ATTRIBUTE_HIDDEN);
#endif

#ifdef Q_OS_MAC
	Q_UNUSED(filePath)
	// .XX is always hidden on Mac
#endif
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
	query.prepare(QObject::tr("update %1 set Name=:newName where Name=:oldName").arg(tableName));
	query.bindValue(":newName", newName);
	query.bindValue(":oldName", oldName);
	query.exec();
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
	query.exec(QObject::tr("select Name from DictionaryTags, PhraseTag \
						   where Phrase=%1 and Tag=ID").arg(phraseID));
	while(query.next())
		tags << query.value(0).toString();
	return tags;
}

// may need to reset the model or call fetchAll first
int idToRow(QAbstractItemModel* model, int idSection, int id)
{
	QModelIndexList indexes = model->match(
		model->index(0, idSection), Qt::DisplayRole, id, 1, Qt::MatchExactly | Qt::MatchWrap);
	return !indexes.isEmpty() ? indexes.at(0).row() : -1;
}

bool phraseExists(const QString& phrase)
{
	QSqlQuery query;
	query.prepare("select * from Dictionary where Phrase = :phrase");
	query.bindValue(":phrase", phrase);
	query.exec();
	return query.next();
}

QString suggestAttachmentName(const QString& fileName)
{
	if(fileName.endsWith(".pdf", Qt::CaseInsensitive))
		return "Paper.pdf";
	else if(fileName.endsWith(".ris", Qt::CaseInsensitive))
		return "EndNote.ris";
	else if(fileName.endsWith(".enw", Qt::CaseInsensitive))
		return "EndNote.enw";
	return QFileInfo(fileName).fileName();
}

void dropTempView()
{
	QSqlQuery query;
	query.exec("drop view SelectedTags");   // remove the temp table
}

Phrase findPhrase(int id)
{
	Phrase result;
	QSqlQuery query;
	query.exec(QObject::tr("select Phrase, Explanation from Dictionary where ID = %1").arg(id));
	if(query.next())
	{
		result.id          = id;
		result.name        = query.value(0).toString();
		result.explanation = query.value(1).toString();
	}
	return result;
}

Phrase findPhrase(const QString& phraseName)
{
	Phrase result;
	QSqlQuery query;
	query.exec(QObject::tr("select ID, Explanation from Dictionary where Phrase = %1").arg(phraseName));
	if(query.next())
	{
		result.name        = phraseName;
		result.id          = query.value(0).toInt();
		result.explanation = query.value(1).toString();
	}
	return result;
}

QStringList splitLine(const QString& line, const QString& sep, 
					  bool removeSpaceInSeparator)
{
	QStringList result;
	QString separator = removeSpaceInSeparator ? sep.trimmed() : sep;
	QStringList sections = line.split(separator);
	foreach(const QString& section, sections)
	{
		QString simplified = section.simplified();
		if(!simplified.isEmpty())
			result << section.simplified();
	}
	return result;
}

QStringList splitNamesLine(const QString& namesLine,
						   const QString& separator,
						   const QString& format)
{
	QStringList result;
	QStringList authorList = splitLine(namesLine, separator, false);
	foreach(const QString& author, authorList)
		result << EnglishName(author).toString(format);
	return result;
}

void centerWindow(QWidget* widget)
{
	QRect screen = QApplication::desktop()->screenGeometry();
	widget->move(screen.center() - widget->rect().center());
}

void fetchAll(QAbstractItemModel* model) {
	while(model->canFetchMore(QModelIndex()))
		model->fetchMore(QModelIndex());
}
