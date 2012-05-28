#ifndef EXPORTER_H
#define EXPORTER_H

#include <QString>
#include <QMap>
#include <QObject>

class Reference;
class RefFormatSpec;

class IRefExporter
{
public:
	virtual ~IRefExporter() {}
	virtual QString toString(const Reference& ref, const RefFormatSpec& spec) const = 0;
};

class NullExporter : public IRefExporter
{
public:
	QString toString(const Reference&, const RefFormatSpec&) const {
		return QObject::tr("Error: Export format not defined!");
    }
};

class LineRefExporter : public IRefExporter
{
public:
	QString toString(const Reference& ref, const RefFormatSpec& spec) const;

private:
    void createLine(QStringList& lines, const QString& lineTemplate,
					const QString& text, const QString& value) const;
};


/////////////////////////////////////////////////////////////////////
class ExporterFactory
{
public:
    static ExporterFactory* getInstance();
    IRefExporter* getExporter(const QString& ext) const;

private:
    ExporterFactory();
    ExporterFactory(const ExporterFactory&) {}
    ExporterFactory& operator=(const ExporterFactory&) { return *this; }
    ~ExporterFactory() {}

private:
    static ExporterFactory* instance;
	QMap<QString, IRefExporter*> exporters;   // extension -> parser
};

#endif // EXPORTER_H
