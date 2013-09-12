#ifndef REFPARSER_H
#define REFPARSER_H

#include "Reference.h"
#include "RefFormatSpec.h"
#include <QList>

class QSettings;

class IRefParser
{
public:
	virtual ~IRefParser() {}
    virtual QList<Reference> parse(const QString& content, RefSpec* spec) = 0;
};

class NullParser : public IRefParser
{
public:
    QList<Reference> parse(const QString&, RefSpec*) { return QList<Reference>(); }
};

// each line is a field, unlike XML
class LineRefParser : public IRefParser
{
public:
    QList<Reference> parse(const QString& content, RefSpec* spec);

private:
	Reference   parseRecord (const QString& record)  const;
	QString     getTypeName (const QString& record)  const;
	QStringList parseAuthors(const QString& authors) const;
	QString     parsePages  (const QString& pages)   const;

protected:
    RefSpec* formatSpec;
};


////////////////////////////////////////////////////////////////
class ParserFactory
{
public:
    static ParserFactory* getInstance();
    IRefParser* getParser(const QString& formatName) const;

private:
    ParserFactory();
    ParserFactory(const ParserFactory&) {}
    ParserFactory& operator=(const ParserFactory&) { return *this; }
    ~ParserFactory() {}

private:
    static ParserFactory* instance;
    QMap<QString, IRefParser*> parsers;   // format -> parser
};


#endif // REFPARSER_H
