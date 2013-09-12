#include "RefExporter.h"
#include "Reference.h"
#include "RefFormatSpec.h"
#include <QStringList>
#include <QTextStream>

QString LineRefExporter::toString(const Reference& ref, const RefSpec& spec) const
{
	// record template
    QString templateRecord = spec.getRecordTemplate();
    QStringList sections = templateRecord.split("#");
    if(sections.size() != 3)
		return QObject::tr("Error: RecordExportTemplate should have 3 sections!\r\n");

    QString lineStart     = sections[0];
    QString lineSeparator = sections[1];
    QString lineEnd       = sections[2];

    QString result;
    QTextStream os(&result);

    // get record type and dictionary
    QString typeName = ref.getValue("type").toString();
    TypeSpec type = spec.getType(typeName);
    if(!type.isValid())
        return QObject::tr("Error: definition for record type \"%1\" not defined!\r\n").arg(typeName);

	// start (type) line
    QStringList lines;
    lineStart.replace("TypeText", spec.getExternalTypeName(typeName));
    lineStart.replace("ID", ref.getValue("id").toString());
    lines << lineStart;

	// field lines
	QString templateField = spec.getFieldTemplate();
    Reference::Fields fields = ref.getAllFields();
    for(Reference::Fields::const_iterator it = fields.begin(); it != fields.end(); ++it)
    {
        QString name = it.key();
        QString text = type.getExternalFieldName(name);
        if(name == "authors")                          // authors is a combo line
        {
            QStringList authors = it.value().toStringList();
			QString separator = spec.getSeparator("authors");
			if(separator.isEmpty()) {   // no separator means one author one line
                foreach(const QString& author, authors)
					createLine(lines, templateField, text, author);
            }
            else {                                      // all authors in one line
				createLine(lines, templateField, text, authors.join(separator));
            }
        }
        else {
			createLine(lines, templateField, text, it.value().toString());
        }
    }
    os << lines.join(lineSeparator + "\r\n");

	// end line
	os << "\r\n" << lineEnd << "\r\n";
    return result;
}

void LineRefExporter::createLine(QStringList& lines, const QString& lineTemplate,
								 const QString& text, const QString& value) const
{
    if(text.isEmpty() || value.isEmpty())
        return;

    QString line = lineTemplate;
    line.replace("FieldName",  text);
    line.replace("FieldValue", value);
    lines << line;
}

////////////////////////////////////////////////////////////////
ExporterFactory* ExporterFactory::instance = 0;

ExporterFactory* ExporterFactory::getInstance()
{
    if(instance == 0)
        instance = new ExporterFactory();
    return instance;
}

IRefExporter *ExporterFactory::getExporter(const QString& ext) const
{
    QString extension = ext.toLower();
    return exporters.contains(extension) ? exporters[extension] : exporters["null"];
}

ExporterFactory::ExporterFactory()
{
    exporters.insert("null", new NullExporter);

    LineRefExporter* lineRefExporter = new LineRefExporter;
    exporters.insert("bib", lineRefExporter);
    exporters.insert("ris", lineRefExporter);
    exporters.insert("enw", lineRefExporter);
}
