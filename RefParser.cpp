#include "RefParser.h"
#include "EnglishName.h"
#include "Common.h"
#include "Convertor.h"
#include <QRegExp>
#include <QSettings>
#include <QFile>
#include <QStringList>

QList<Reference> LineRefParser::parse(const QString& content, RefFormatSpec* spec)
{
	QList<Reference> results;
    if(content.isEmpty() || spec == 0)
        return results;
    formatSpec = spec;

    QRegExp rxRecordStart(formatSpec->getTypePattern());  // always start with type
	int idxRecordStart = rxRecordStart.indexIn(content);
	while(idxRecordStart > -1)                            // parse each record
	{
		int idxLastStart = idxRecordStart;
		idxRecordStart = rxRecordStart.indexIn(content, idxLastStart + rxRecordStart.matchedLength());
        QString record = content.mid(idxLastStart, idxRecordStart - idxLastStart).trimmed();
        results << parseRecord(record);
	}

    return results;
}

QString LineRefParser::getType(const QString& record) const
{
	// check type pattern
    if(formatSpec->getTypePattern().isEmpty())
        return QString();

    QString type;
    QRegExp rxType(formatSpec->getTypePattern());
	int idxType = rxType.indexIn(record);
	if(idxType > -1)
	{
        QString typeText = rxType.cap(1).simplified();  // capture type text
        type = formatSpec->getTypeName(typeText);       // convert to type name
	}
	return type;
}

Reference LineRefParser::parseRecord(const QString& record) const
{
    Reference result;
	if(record.isEmpty())
        return result;

	// get record type
    QString type = getType(record);
	if(type.isEmpty())
        return result;
	result.setValue("type", type);

    // get dictionary for this type
    FieldDictionary* dictionary = formatSpec->getFieldDictionary(type);
	if(dictionary == 0)   // this type not defined in the spec
		return result;

	// parse each field;
    QRegExp rxField(formatSpec->getFieldPattern());
	int idxField = rxField.indexIn(record);
	while(idxField > -1)
	{
        QString fieldText = rxField.cap(1).simplified();
        QString fieldName = dictionary->getName(fieldText);  // field text -> field name
        if(!fieldName.isEmpty())
        {
            QString fieldValue = rxField.cap(2).simplified();

            // special fields
            if(fieldName == "title")
                result.setValue(fieldName, CaseConvertor().convert(fieldValue));
            else if(fieldName == "authors")
                result.setValue(fieldName, parseAuthors(fieldValue));
			else if(fieldName == "pages")
                result.setValue(fieldName, parsePages(fieldValue));
            else
                result.setValue(fieldName, fieldValue);
        }
		idxField = rxField.indexIn(record, idxField + rxField.matchedLength());
	}

    // generate id
    if(formatSpec->getRecordTemplate().contains("ID"))
        result.generateID();

    return result;
}

QStringList LineRefParser::parseAuthors(const QString& authors) const
{
    QStringList result;
    QString separator = formatSpec->getAuthorsSeparator();
	if(separator.isEmpty())   // one line is one author
        result << authors;
	else                      // all authors in one line
		result << splitAuthorsList(authors, separator);
    return result;
}

QString LineRefParser::parsePages(const QString& pages) const
{
    QString startPage("0"), endPage("0");
    QRegExp rx("(\\d+)");
    int idx = rx.indexIn(pages);
    if(idx > -1)
    {
        startPage = endPage = rx.cap(1);
        if(rx.indexIn(pages, idx + rx.matchedLength()) > -1)
            endPage = rx.cap(1);
    }
    return startPage + "-" + endPage;
}


////////////////////////////////////////////////////////////////
ParserFactory* ParserFactory::instance = 0;

ParserFactory* ParserFactory::getInstance()
{
    if(instance == 0)
        instance = new ParserFactory();
    return instance;
}

IRefParser* ParserFactory::getParser(const QString& ext) const
{
    QString extension = ext.toLower();
    return parsers.contains(extension) ? parsers[extension] : parsers["null"];
}

ParserFactory::ParserFactory()
{
    parsers.insert("null", new NullParser);

    LineRefParser* lineRefParser = new LineRefParser;
    parsers.insert("bib",  lineRefParser);
    parsers.insert("ris",  lineRefParser);
    parsers.insert("enw",  lineRefParser);
}
