#include "RefFormatSpec.h"
#include <QFile>
#include <QSettings>
#include <QStringList>

bool DoubleMap::contains1(const QString& t1) const {
    return !value2(t1).isEmpty();
}

bool DoubleMap::contains2(const QString& t2) const {
    return !value1(t2).isEmpty();
}

QString DoubleMap::value1(const QString& t2) const
{
    foreach(const Pair& pair, list)
        if(pair.second.compare(t2, Qt::CaseInsensitive) == 0)
            return pair.first;
    return QString();
}

QString DoubleMap::value2(const QString& t1) const
{
    foreach(const Pair& pair, list)
        if(pair.first.compare(t1, Qt::CaseInsensitive) == 0)
            return pair.second;
    return QString();
}


/////////////////////////////////////////////////////////////////////////
void FieldDictionary::insert(const QString& text, const QString& name) {
    map.insert(text, name);
}

QString FieldDictionary::getName(const QString& text) const {
	return map.value2(text);
}

QString FieldDictionary::getText(const QString& name) const {
	return map.value1(name);
}


//////////////////////////////////////////////////////////////////////
bool RefFormatSpec::load(const QString& ext)
{
    clear();

    QString extension = ext.toLower();
    QString defFileName = extension + "Specification.ini";
    if(!QFile::exists(defFileName))
        return false;

    QSettings defFile(defFileName, QSettings::IniFormat);

    // load patterns and templates
    patternType = defFile.value("TypePattern").toString();
    if(patternType.isEmpty())
        return false;

    patternField = defFile.value("FieldPattern").toString();
    if(patternField.isEmpty())
        return false;

    templateRecord = defFile.value("RecordExportTemplate").toString();
    if(templateRecord.isEmpty())
        return false;

    templateField = defFile.value("FieldExportTemplate").toString();
    if(templateField.isEmpty())
        return false;

	// separators are optional
	separatorAuthors = defFile.value("AuthorsSeparator").toString();
	separatorPages   = defFile.value("PagesSeparator")  .toString();

    // load type definitions
    QStringList typeNames = defFile.childGroups();
    foreach(const QString& typeName, typeNames)
    {
        defFile.beginGroup(typeName);

        // type text
        QString typeText = defFile.value("TypeText").toString();
        if(typeText.isEmpty())
            return false;

        // create field dictionary for this type
        if(fieldDictionaries.contains(typeName))   // this type already defined
        {
			defFile.endGroup();  // NOTE: don't forget to exit current group
            continue;
        }
        FieldDictionary* fieldDictionary = new FieldDictionary;

		// add field definitions to the dictionary
        QStringList fieldTexts = defFile.childKeys();
        foreach(const QString& fieldText, fieldTexts)
        {
            QString fieldName = defFile.value(fieldText).toString();
            if(fieldText != "TypeText")    // ignore TypeKeyword, which is also a key
                fieldDictionary->insert(fieldText, fieldName);
        }

        addType(typeText, typeName, fieldDictionary);
        defFile.endGroup();
    }
    return true;
}

void RefFormatSpec::clear()
{
	patternType     .clear();
	patternField    .clear();
	templateRecord  .clear();
	templateField   .clear();
	separatorAuthors.clear();
	separatorPages  .clear();
	typeDictionary  .clear();
    foreach(FieldDictionary* dictionary, fieldDictionaries)
        delete dictionary;
    fieldDictionaries.clear();
}

void RefFormatSpec::addType(const QString& text, const QString& name, FieldDictionary* dictionary)
{
    typeDictionary.insert(text, name);
    fieldDictionaries.insert(name, dictionary);
}

QString RefFormatSpec::getTypeName(const QString& typeText) const {
    return typeDictionary.value2(typeText);
}

QString RefFormatSpec::getTypeText(const QString& typeName) const {
    return typeDictionary.value1(typeName);
}

FieldDictionary* RefFormatSpec::getFieldDictionary(const QString& typeName) const {
    return fieldDictionaries.contains(typeName) ? fieldDictionaries[typeName] : 0;
}


////////////////////////////////////////////////////////////////////////////////
SpecFactory* SpecFactory::instance = 0;

SpecFactory* SpecFactory::getInstance()
{
    if(instance == 0)
        instance = new SpecFactory();
    return instance;
}

RefFormatSpec* SpecFactory::getSpec(const QString& ext)
{
    // spec exists
    QString extension = ext.toLower();
    if(specs.contains(extension))
        return specs[extension];

    // load a new spec
    RefFormatSpec* spec = new RefFormatSpec;
    if(spec->load(extension))
    {
        specs.insert(extension, spec);
        return spec;
    }
    return 0;
}
