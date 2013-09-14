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

    bool      fieldExistsByExternalName(const QString& exName) const;
    bool      fieldExistsByInternalName(const QString& inName) const;

    FieldSpec getFieldByExternalName   (const QString& exName) const;
    FieldSpec getFieldByInternalName   (const QString& inName) const;

    QString   getExternalFieldName(const QString& inFieldName) const;  // convenience
    QString   getInternalFieldName(const QString& exFieldName) const;

    QString   getExternalName() const { return _externalName; }
    QString   getInternalName() const { return _internalName; }

    bool      isRequiredField(const QString& inFieldName) const;
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
    bool load(const QString& specName);
    QString getTypePattern()      const { return _patternType;      }
    QString getFieldPattern()     const { return _patternField;     }

    QString getRecordTemplate()   const { return _templateRecord;   }
    QString getFieldTemplate()    const { return _templateField;    }

	QString getSeparator(const QString& inName) const;

    QString getInternalTypeName(const QString& exTypeName) const;
    QString getExternalTypeName(const QString& inTypeName) const;
    IRefParser* getParser() const;   // every spec relates to a parser

    TypeSpec getType(const QString& inTypeName) const;
    QMap<QString, TypeSpec> getAllTypes() const { return _types; }

private:
    void loadType     (QXmlStreamReader& xml);
	void loadSeparator(QXmlStreamReader& xml);

private:
    QString                 _specName;
    QMap<QString, TypeSpec> _types;   // internalName->TypeSpec

    // global properties
    QString _patternType;       // also indicates start of the record
    QString _patternField;
    QString _templateRecord;
    QString _templateField;
	QMap<QString, QString> _separators;  // inName->separator
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
