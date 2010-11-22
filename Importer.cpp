#include "Importer.h"
#include <QFile>
#include <QTextStream>
#include <QtXml>

Importer* ImporterFactory::getImporter(const QString& fileName)
{
	if(fileName.endsWith(".enw", Qt::CaseInsensitive))
		return new EnwImporter();
	if(fileName.endsWith(".ris", Qt::CaseInsensitive))
		return new RisImporter();
	if(fileName.endsWith(".xml", Qt::CaseInsensitive))
		return new XmlImporter();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool NonXmlImporter::import(const QString& fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return false;

	QTextStream is(&file);
	Results::Iterator currentResult;
	while(!is.atEnd())
	{
		QString line = is.readLine();
		if(line.startsWith(getFirstHead()))
		{
			results << ImportResult();
			currentResult = results.end() - 1;   // the last
			continue;
		}

		foreach(QString titleHead, getTitleHeads())
			if(line.startsWith(titleHead))
			{
				currentResult->title = trimHead(line, titleHead);
				continue;
			}

		foreach(QString authorHead, getAuthorHeads())
			if(line.startsWith(authorHead))
			{
				QString author = trimHead(line, authorHead);
				if(currentResult->authors.isEmpty())
					currentResult->authors = author;
				else
					currentResult->authors.append("; " + author);
				continue;
			}

		foreach(QString journalHead, getJournalHeads())
			if(line.startsWith(journalHead))
			{
				QString journal = trimHead(line, journalHead);
				if(currentResult->journal.isEmpty())
					currentResult->journal = journal;
				else
					currentResult->journal.append("; " + journal);
				continue;
			}

		foreach(QString yearHead, getYearHeads())
			if(line.startsWith(yearHead))
			{
				currentResult->year = trimHead(line, yearHead).toInt();
				continue;
			}

		foreach(QString abstractHead, getAbstractHeads())
			if(line.startsWith(abstractHead))
			{
				currentResult->abstract = trimHead(line, abstractHead);
				continue;
			}
	}
	return true;
}

QString NonXmlImporter::trimHead(const QString& line, const QString head) const {
	return line.mid(line.indexOf(head) + head.length()).trimmed();
}

//////////////////////////////////////////////////////////////////////////
QString EnwImporter::getFirstHead() const {
	return "%0 ";
}
QStringList EnwImporter::getTitleHeads() const {
	return QStringList() << "%T ";
}
QStringList EnwImporter::getAuthorHeads() const {
	return QStringList() << "%A ";
}
QStringList EnwImporter::getJournalHeads() const {
	return QStringList() << "%J " << "%B ";
}
QStringList EnwImporter::getYearHeads() const {
	return QStringList() << "%D ";
}
QStringList EnwImporter::getAbstractHeads() const {
	return QStringList();
}

//////////////////////////////////////////////////////////////////////////
QString RisImporter::getFirstHead() const {
	return "TY  -";
}
QStringList RisImporter::getTitleHeads() const {
	return QStringList() << "TI  -" << "T1  -";
}
QStringList RisImporter::getAuthorHeads() const {
	return QStringList() << "AU  -";
}
QStringList RisImporter::getJournalHeads() const {
	return QStringList() << "JA  -" << "T2  -" << "T3  -";
}
QStringList RisImporter::getYearHeads() const {
	return QStringList() << "PY  -";
}
QStringList RisImporter::getAbstractHeads() const {
	return QStringList() << "AB  -";
}

//////////////////////////////////////////////////////////////////////////
bool XmlImporter::import(const QString& fileName)
{
	QDomDocument doc;
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly))
		return false;
	if(!doc.setContent(file.readAll().toLower()))
	{
		file.close();
		return false;
	}
	file.close();

	QDomNodeList records = doc.elementsByTagName("record");
	for(int i=0; i<records.count(); ++i)
	{
		QDomNode record = records.item(i);
		ImportResult result;

		QDomNodeList title = record.toElement().elementsByTagName("title");
		if(title.isEmpty())
			return false;
		result.title = title.item(0).toElement().text();

		QDomNodeList authors = record.toElement().elementsByTagName("author");
		for(int j=0; j<authors.count(); ++j)
		{
			QDomNode author = authors.item(j);
			if(result.authors.isEmpty())
				result.authors = author.toElement().text();
			else
				result.authors.append("; " + author.toElement().text());
		}

		QDomNodeList journal = record.toElement().elementsByTagName("secondary-title");
		if(!journal.isEmpty())
		{
			if(result.journal.isEmpty())
				result.journal = journal.item(0).toElement().text();
			else
				result.journal.append("; " + journal.item(0).toElement().text());
		}

		journal = record.toElement().elementsByTagName("alt-title");
		if(!journal.isEmpty())
		{
			if(result.journal.isEmpty())
				result.journal = journal.item(0).toElement().text();
			else
				result.journal.append("; " + journal.item(0).toElement().text());
		}

		QDomNodeList year = record.toElement().elementsByTagName("year");
		if(!year.isEmpty())
			result.year = year.item(0).toElement().text().toInt();

		QDomNodeList abstract = record.toElement().elementsByTagName("abstract");
		if(!abstract.isEmpty())
			result.abstract = abstract.item(0).toElement().text();

		results << result;
	}
	return true;
}
