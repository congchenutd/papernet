#ifndef CASECONVERTOR_H
#define CASECONVERTOR_H

#include <QObject>
#include <QStringList>

class Convertor : public QObject
{
	Q_OBJECT

public:
	Convertor(QObject* parent = 0) : QObject(parent) {}
	virtual ~Convertor() {}
	virtual QString convert(const QString& input) const = 0;

signals:
	void converted(const QString& input, const QString& output) const;
};


//////////////////////////////////////////////////////////////
class CaseConvertor : public Convertor
{
	Q_OBJECT
public:
	CaseConvertor(QObject* parent = 0);
	virtual QString convert(const QString& input) const;

private:
	QString toFirstCharUpperCase(const QString& word) const;

private:
	QStringList lowercaseWords;
};


///////////////////////////////////////////////////////////////
class ProtectionConvertor : public Convertor
{
	Q_OBJECT
public:
	ProtectionConvertor(QObject* parent = 0);
	virtual QString convert(const QString& input) const;

private:
	QString toFirstCharProtected(const QString& word) const;

private:
	QStringList lowercaseWords;
};


///////////////////////////////////////////////////////////////
//class AbbreviationConvertor : public Convertor
//{
//public:
//	AbbreviationConvertor(QObject* parent = 0);
//	virtual QString convert(const QString& input) const;

//private:
//	QStringList rules;
//};

#endif // CASECONVERTOR_H
