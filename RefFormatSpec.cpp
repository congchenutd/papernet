#include "RefFormatSpec.h"
#include "RefParser.h"
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QDir>
#include <QXmlStreamReader>

Type::Type(const QString& externalName, const QString& internalName)
    : _externalName(externalName), _internalName(internalName)
{}

void Type::addField(const QString& externalName, const QString& internalName, bool required)
{
    if(!fieldExistsByExternalName(externalName) && !fieldExistsByInternalName(internalName))
        _fields << Field(externalName, internalName, required);
}

bool Type::fieldExistsByExternalName(const QString& externalName) const {
    return !getFieldByExternalName(externalName)._externalName.isEmpty();
}

bool Type::fieldExistsByInternalName(const QString& internalName) const {
    return !getFieldByInternalName(internalName)._internalName.isEmpty();
}

Field Type::getFieldByExternalName(const QString& externalName) const
{
    foreach(const Field& field, _fields)
        if(field._externalName.compare(externalName, Qt::CaseInsensitive) == 0)
            return field;
    return Field();
}

Field Type::getFieldByInternalName(const QString& internalName) const
{
    foreach(const Field& field, _fields)
        if(field._internalName.compare(internalName, Qt::CaseInsensitive) == 0)
            return field;
    return Field();
}

QString Type::getExternalFieldName(const QString& internalFieldName) const {
    return getFieldByInternalName(internalFieldName)._externalName;
}

QString Type::getInternalFieldName(const QString& externalFieldName) const {
    return getFieldByExternalName(externalFieldName)._internalName;
}


/////////////////////////////////////////////////////////////////////////////
bool RefFormatSpec::load(const QString& format)
{
    // reset
    _formatName      .clear();
    _patternType     .clear();
    _patternField    .clear();
    _templateRecord  .clear();
    _templateField   .clear();
    _separatorAuthors.clear();
    _separatorPages  .clear();
    _formatName = format;

    // open file
    QFile file("./Specifications/" + _formatName + ".xml");
    if(!file.open(QFile::ReadOnly))
        return false;

    // root node
    QXmlStreamReader xml(&file);
    if(!xml.readNextStartElement() || xml.name() != "spec")
        return false;

    // load the rest nodes
    while(!xml.atEnd())
        if(xml.readNextStartElement())
        {
            // global attributes
            QString name = xml.name().toString();
            if(name == "TypePattern")
                _patternType = xml.readElementText();
            else if(name == "FieldPattern")
                _patternField = xml.readElementText();
            else if(name == "RecordExportTemplate")
                _templateRecord = xml.readElementText();
            else if(name == "FieldExportTemplate")
                _templateField = xml.readElementText();
            else if(name == "AuthorsSeparator")
                _separatorAuthors = xml.readElementText();
            else if(name == "PagesSeparator")
                _separatorPages = xml.readElementText();

            // types
            else if(name == "type")
                loadType(xml);
        }
    return true;
}

void RefFormatSpec::loadType(QXmlStreamReader& xml)
{
    if(!xml.isStartElement() || xml.name() != "type")
        return;

    // type names
    QString externalTypeName = xml.attributes().value("external").toString();
    QString internalTypeName = xml.attributes().value("internal").toString();
    Type type(externalTypeName, internalTypeName);

    // fields
    while(!(xml.isEndElement() && xml.name() == "type"))  // until </type>
        if(xml.readNextStartElement() && xml.name() == "field")
        {
            QString externalFieldName = xml.attributes().value("external").toString();
            QString internalFieldName = xml.attributes().value("internal").toString();
            bool    required = xml.attributes().value("required").toString().toLower() == "yes";
            type.addField(externalFieldName, internalFieldName, required);
        }

    // even this type already exists, still need to read (pass) this part of the xml
    if(!typeExists(internalTypeName))
        _types << type;
}

bool RefFormatSpec::typeExists(const QString& internalTypeName) const {
    return !getType(internalTypeName).getInternalName().isEmpty();
}

Type RefFormatSpec::getType(const QString& internalTypeName) const
{
    foreach(const Type& type, _types)
        if(type.getInternalName().compare(internalTypeName, Qt::CaseInsensitive) == 0)
            return type;
    return Type();
}

QString RefFormatSpec::getInternalTypeName(const QString& externalTypeName) const
{
    foreach(const Type& type, _types)
        if(type.getExternalName().compare(externalTypeName, Qt::CaseInsensitive) == 0)
            return type.getInternalName();
    return QString();
}

QString RefFormatSpec::getExternalTypeName(const QString& internalTypeName) const
{
    foreach(const Type& type, _types)
        if(type.getInternalName().compare(internalTypeName, Qt::CaseInsensitive) == 0)
            return type.getExternalName();
    return QString();
}

IRefParser* RefFormatSpec::getParser() const {
    return ParserFactory::getInstance()->getParser(_formatName);
}


////////////////////////////////////////////////////////////////////////////////
SpecFactory* SpecFactory::instance = 0;

SpecFactory* SpecFactory::getInstance()
{
    if(instance == 0)
        instance = new SpecFactory();
    return instance;
}

RefFormatSpec* SpecFactory::getSpec(const QString& format)
{
    // spec exists
    QString fmt = format.toLower();
    if(specs.contains(fmt))
        return specs[fmt];

    // load a new spec
    RefFormatSpec* spec = new RefFormatSpec;
    if(spec->load(fmt))
    {
        specs.insert(fmt, spec);
        return spec;
    }
    delete spec;
    return 0;
}

QList<Reference> SpecFactory::parseContent(const QString& content)
{
    QFileInfoList infos = QDir("./Specifications").entryInfoList(QStringList() << "*.spec");
    foreach(QFileInfo info, infos)
        if(RefFormatSpec* spec = getSpec(info.baseName()))
        {
            QList<Reference> references = spec->getParser()->parse(content, spec);
            if(!references.isEmpty())
                return references;
        }
    return QList<Reference>();
}

