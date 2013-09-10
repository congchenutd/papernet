#ifndef REFFORMATSPEC_H
#define REFFORMATSPEC_H

#include <QList>
#include <QString>
#include <QPair>
#include <QMap>
#include "Reference.h"

// defines a field
struct FieldSpec
{
    FieldSpec(const QString& externalName = QString(),
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

// defines a ref type
// consisting of an external type name, an internal type name, and several fields
class TypeSpec
{
public:
    TypeSpec(const QString& externalName = QString(), const QString& internalName = QString());

    bool isValid() const { return !_externalName.isEmpty() && !_internalName.isEmpty(); }
    void addField(const QString& externalName, const QString& internalName, bool required = false);

    bool      fieldExistsByExternalName(const QString& externalName) const;
    bool      fieldExistsByInternalName(const QString& internalName) const;
    FieldSpec getFieldByExternalName   (const QString& externalName) const;
    FieldSpec getFieldByInternalName   (const QString& internalName) const;
    QString   getExternalFieldName(const QString& internalFieldName) const;
    QString   getInternalFieldName(const QString& externalFieldName) const;
    QString   getExternalName() const { return _externalName; }
    QString   getInternalName() const { return _internalName; }
    bool      isRequiredField(const QString& fieldName) const;
    QList<FieldSpec> getAllFields() const { return _fields; }

private:
    QString _externalName;
    QString _internalName;
    QList<FieldSpec> _fields;
};

class IRefParser;
class QXmlStreamReader;

// Specification of a bib format (bib, ris, enw, etc.)
// consists of global properties and types
// each type consists of an external name, an internal name, and fields
// each field has an external name, an internal name, and if required
class RefSpec
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

    bool     typeExists(const QString& internalTypeName) const;
    TypeSpec getType   (const QString& internalTypeName) const;
    QMap<QString, TypeSpec> getAllTypes() const { return _types; }

private:
    void loadType(QXmlStreamReader& xml);
    TypeSpec makeDefaultTypeSpec() const;

private:
    QString                 _formatName;
    QMap<QString, TypeSpec> _types;

    // global properties
    QString _patternType;       // also indicates start of the record
    QString _patternField;
    QString _templateRecord;
    QString _templateField;
    QString _separatorAuthors;  // empty means one author one line
    QString _separatorPages;    // empty means startpage and endpage, instead of pages
};


////////////////////////////////////////////////////////////
/// map format (file extension) to RefSpec
class RefSpecFactory
{
public:
    static RefSpecFactory* getInstance();
    RefSpec* getSpec(const QString& extension);

private:
    RefSpecFactory() {}
    RefSpecFactory(const RefSpecFactory&) {}
    RefSpecFactory& operator=(const RefSpecFactory&) { return *this; }
    ~RefSpecFactory() {}

private:
    static RefSpecFactory* instance;
    QMap<QString, RefSpec*> specs;     // format -> spec
};


#endif // REFFORMATSPEC_H
