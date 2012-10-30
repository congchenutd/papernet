#ifndef REFFORMATSPEC_H
#define REFFORMATSPEC_H

#include <QList>
#include <QString>
#include <QPair>
#include <QMap>
#include "Reference.h"

// A case-insensitive double map
class DoubleMap
{
public:
    void clear() { list.clear(); }
    void insert(const QString& t1, const QString& t2)  { list << Pair(t1, t2); }

    bool contains1(const QString& t1) const;
    bool contains2(const QString& t2) const;

    QString value1(const QString& t2) const;
    QString value2(const QString& t1) const;

private:
    typedef QPair<QString, QString> Pair;
    QList<Pair> list;
};

// field text <-> field name
// an adapter
class FieldDictionary
{
public:
    void insert(const QString& text, const QString& name);
    QString getName(const QString& text) const;
    QString getText(const QString& name) const;

private:
    DoubleMap map;
};

class IRefParser;

// Specification of a bib format (bib, ris, enw, etc)
// consists of global properties and reference type specifications
class RefFormatSpec
{
public:
    bool load(const QString& format);
    QString getTypePattern()      const { return patternType;      }
    QString getFieldPattern()     const { return patternField;     }
    QString getRecordTemplate()   const { return templateRecord;   }
    QString getFieldTemplate()    const { return templateField;    }
    QString getAuthorsSeparator() const { return separatorAuthors; }
    QString getPagesSeparator()   const { return separatorPages;   }
    QString getTypeName(const QString& typeText) const;
    QString getTypeText(const QString& typeName) const;
    FieldDictionary* getFieldDictionary(const QString& typeName) const;
    IRefParser*      getParser() const;
    QList<Reference> parse(const QString& content);

private:
    void clear();
    void addType(const QString& text, const QString& name, FieldDictionary* dictionary);

private:
    QString       formatName;
	DoubleMap     typeDictionary;                       // Type text -> type name
    QMap<QString, FieldDictionary*> fieldDictionaries;  // Type name -> field dictionary

    // for import
    QString patternType;     // also indicates the record start
    QString patternField;

    // for export
    QString templateRecord;
    QString templateField;

	QString separatorAuthors;  // empty means one author one line
	QString separatorPages;    // empty means startpage and endpage, instead of pages
};


////////////////////////////////////////////////////////
class SpecFactory
{
public:
    static SpecFactory* getInstance();
    RefFormatSpec* getSpec(const QString& format);
    QList<Reference> parseContent(const QString& content);

private:
    SpecFactory() {}
    SpecFactory(const SpecFactory&) {}
    SpecFactory& operator=(const SpecFactory&) { return *this; }
    ~SpecFactory() {}

private:
    static SpecFactory* instance;
    QMap<QString, RefFormatSpec*> specs;     // format -> spec
};


#endif // REFFORMATSPEC_H
