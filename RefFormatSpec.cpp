#include "RefFormatSpec.h"
#include "RefParser.h"
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QDir>
#include <QXmlStreamReader>

TypeSpec::TypeSpec(const QString& exName, const QString& inName)
    : _externalName(exName), _internalName(inName)
{}

void TypeSpec::addField(const QString& exName, const QString& inName, bool required)
{
    if(!fieldExistsByExternalName(exName) && !fieldExistsByInternalName(inName))
        _fields << FieldSpec(exName, inName, required);
}

bool TypeSpec::fieldExistsByExternalName(const QString& exName) const {
    return !getFieldByExternalName(exName)._externalName.isEmpty();
}
bool TypeSpec::fieldExistsByInternalName(const QString& inName) const {
    return !getFieldByInternalName(inName)._internalName.isEmpty();
}

FieldSpec TypeSpec::getFieldByExternalName(const QString& exName) const
{
    foreach(const FieldSpec& field, _fields)
        if(field._externalName.compare(exName, Qt::CaseInsensitive) == 0)
            return field;
    return FieldSpec();
}
FieldSpec TypeSpec::getFieldByInternalName(const QString& inName) const
{
    foreach(const FieldSpec& field, _fields)
        if(field._internalName.compare(inName, Qt::CaseInsensitive) == 0)
            return field;
    return FieldSpec();
}

QString TypeSpec::getExternalFieldName(const QString& inFieldName) const {
    return getFieldByInternalName(inFieldName)._externalName;
}
QString TypeSpec::getInternalFieldName(const QString& exFieldName) const {
    return getFieldByExternalName(exFieldName)._internalName;
}

bool TypeSpec::isRequiredField(const QString& inFieldName) const
{
    FieldSpec field = getFieldByInternalName(inFieldName);
    return field.isValid() ? field._required : false;
}


/////////////////////////////////////////////////////////////////////////////
bool RefSpec::load(const QString& specName)
{
    // reset
    _patternType     .clear();
    _patternField    .clear();
    _templateRecord  .clear();
    _templateField   .clear();
    _specName = specName;

    // open file
    QFile file("./Specifications/" + _specName + ".xml");
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
            else if(name == "Separator")
				loadSeparator(xml);

            // types
            else if(name == "type")
                loadType(xml);
        }

	// unknown is a null object
	_types.insert("unknown", TypeSpec("unknown", "unknown"));

    return true;
}

void RefSpec::loadSeparator(QXmlStreamReader& xml)
{
    if(!xml.isStartElement() || xml.name() != "Separator")
		return;

	QString inName    = xml.attributes().value("internal").toString();
	QString separator = xml.readElementText();
	_separators.insert(inName, separator);
}

void RefSpec::loadType(QXmlStreamReader& xml)
{
    if(!xml.isStartElement() || xml.name() != "type")
        return;

    // type names
    QString exTypeName = xml.attributes().value("external").toString();
    QString inTypeName = xml.attributes().value("internal").toString();
    TypeSpec type(exTypeName, inTypeName);

    // fields
    while(!(xml.isEndElement() && xml.name() == "type"))  // until </type>
        if(xml.readNextStartElement() && xml.name() == "field")
        {
            QString exFieldName = xml.attributes().value("external").toString();
            QString inFieldName = xml.attributes().value("internal").toString();
            bool    required = xml.attributes().value("required").toString().toLower() == "yes";
            type.addField(exFieldName, inFieldName, required);
        }

    // even this type already exists, still need to read (pass) this part of the xml
    if(!_types.contains(inTypeName))
        _types.insert(inTypeName, type);
}

TypeSpec RefSpec::getType(const QString& inTypeName) const {
    return _types.contains(inTypeName) ? _types[inTypeName] : _types["unknown"];
}

QString RefSpec::getInternalTypeName(const QString& exTypeName) const
{
    foreach(const TypeSpec& type, _types)
        if(type.getExternalName().compare(exTypeName, Qt::CaseInsensitive) == 0)
            return type.getInternalName();
    return QString("unknown");
}

QString RefSpec::getExternalTypeName(const QString& inTypeName) const {
    return getType(inTypeName).getExternalName();
}

IRefParser* RefSpec::getParser() const {
    return ParserFactory::getInstance()->getParser(_specName);
}

QString RefSpec::getSeparator(const QString& inName) const {
	return _separators.contains(inName) ? _separators[inName] : QString();
}

////////////////////////////////////////////////////////////////////////////////
RefSpecFactory* RefSpecFactory::instance = 0;

RefSpecFactory* RefSpecFactory::getInstance()
{
    if(instance == 0)
        instance = new RefSpecFactory();
    return instance;
}

RefSpec* RefSpecFactory::getSpec(const QString& extension)
{
    // spec exists
    QString fmt = extension.toLower();
    if(specs.contains(fmt))
        return specs[fmt];

    // load a new spec
    RefSpec* spec = new RefSpec;
    if(spec->load(fmt))
    {
        specs.insert(fmt, spec);
        return spec;
    }
    delete spec;
    return 0;
}

