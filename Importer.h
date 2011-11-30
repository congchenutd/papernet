#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QList>
#include <QStringList>

// A family of citation file importers

class Importer;
class ImporterFactory
{
public:
	static Importer* getImporter(const QString& fileName);
};

struct PaperRecord
{
	QString title;
	QString authors;
	QString journal;
	int     year;
	QString abstract;
	QString note;      // not for Importer
	QStringList tags;
};

class Importer : public QObject
{
protected:
	typedef QList<PaperRecord> Results;

public:
	virtual ~Importer() {}

	virtual bool import(const QString& fileName) = 0;
	Results getResults() { return results; }

protected:
	Results results;
};

class NonXmlImporter : public Importer
{
public:
	virtual bool import(const QString& fileName);

protected:
	virtual QString     getFirstHead()     const = 0;
	virtual QStringList getTitleHeads()    const = 0;
	virtual QStringList getAuthorHeads()   const = 0;
	virtual QStringList getJournalHeads()  const = 0;
	virtual QStringList getYearHeads()     const = 0;
	virtual QStringList getAbstractHeads() const = 0;

private:
	QString trimHead(const QString& line, const QString head) const;
};

class EnwImporter : public NonXmlImporter
{
protected:
	virtual QString     getFirstHead()     const;
	virtual QStringList getTitleHeads()    const;
	virtual QStringList getAuthorHeads()   const;
	virtual QStringList getJournalHeads()  const;
	virtual QStringList getYearHeads()     const;
	virtual QStringList getAbstractHeads() const;
};

class RisImporter : public NonXmlImporter
{
protected:
	virtual QString     getFirstHead()     const;
	virtual QStringList getTitleHeads()    const;
	virtual QStringList getAuthorHeads()   const;
	virtual QStringList getJournalHeads()  const;
	virtual QStringList getYearHeads()     const;
	virtual QStringList getAbstractHeads() const;
};

class XmlImporter : public Importer
{
public:
	virtual bool import(const QString& fileName);
};

#endif // IMPORTER_H

