#ifndef REFFORMATSPEC_H
#define REFFORMATSPEC_H

#include <QList>
#include <QString>
#include <QPair>
#include <QMap>
#include "Reference.h"

struct Field
{
    Field(const QString& externalName = QString(),
          const QString& internalName = QString(),
          bool required = false)
        : _externalName(externalName),
          _internalName(internalName),
          _required(required)
    {}

    bool isValid() const { return !_externalName.isEmpty() && !_internalName.isEmpty(); }

    QString _externalName;
    QString _internalName;
    bool    _required;
};

class Type
{
public:
    Type(const QString& externalName = QString(), const QString& internalName = QString());

    bool isValid() const { return !_externalName.isEmpty() && !_internalName.isEmpty(); }
    void addField(const QString& externalName, const QString& internalName, bool required = false);

    bool    fieldExistsByExternalName(const QString& externalName) const;
    bool    fieldExistsByInternalName(const QString& internalName) const;
    Field   getFieldByExternalName   (const QString& externalName) const;
    Field   getFieldByInternalName   (const QString& internalName) const;
    QString getExternalFieldName(const QString& internalFieldName) const;
    QString getInternalFieldName(const QString& externalFieldName) const;
    QString getExternalName() const { return _externalName; }
    QString getInternalName() const { return _internalName; }

private:
    QString _externalName;
    QString _internalName;
    QList<Field> _fields;
};

class IRefParser;
class QXmlStreamReader;

// Specification of a bib format (bib, ris, enw, etc.)
// consists of global properties and types
// each type consists of an external name, an internal name, and fields
// each field has an external name, an internal name, and if required
class RefFormatSpec
{
public:
    bool load(const QString& format);
    QString getTypePattern()      const { return _patternType;      }
    QString getFieldPattern()     const { return _patternField;     }
    QString getRecordTemplate()   const { return _templateRecord;   }
    QString getFieldTemplate()    const { return _templateField;    }
    QString getAuthorsSeparator() const { return _separatorAuthors; }
    QString getPagesSeparator()   const { return _separatorPages;   }
    QString getInternalTypeName(const QString& externalTypeName) const;
    QString getExternalTypeName(const QString& internalTypeName) const;
    IRefParser* getParser() const;   // every spec relates to a parser

    bool typeExists(const QString& internalTypeName) const;
    Type getType   (const QString& internalTypeName) const;

private:
    void loadType(QXmlStreamReader& xml);

private:
    QString     _formatName;
    QList<Type> _types;

    // global properties
    QString _patternType;       // also indicates the record start
    QString _patternField;
    QString _templateRecord;
    QString _templateField;
    QString _separatorAuthors;  // empty means one author one line
    QString _separatorPages;    // empty means startpage and endpage, instead of pages
};


////////////////////////////////////////////////////////
class SpecFactory
{
public:
    static SpecFactory* getInstance();
    RefFormatSpec* getSpec(const QString& format);
    QList<Reference> parseContent(const QString& content);  // parse using any possible specs

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
