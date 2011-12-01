#ifndef THESAURUS_H
#define THESAURUS_H

#include "ThesaurusCache.h"
#include <QObject>
#include <QUrl>
class QNetworkAccessManager;
class QNetworkReply;

// An interface for online thesaurus service
class Thesaurus : public QObject
{
	Q_OBJECT

public:
	Thesaurus(QObject* parent = 0);
	void request(const QString& word);

signals:
	void response(const QStringList& results);

protected slots:
	virtual void parse(QNetworkReply* reply) = 0;  // template methods

protected:
	virtual QUrl makeUrl(const QString& word) const = 0;   // make the url for the request
	void updateCache(const QStringList& words);

protected:
	QNetworkAccessManager* networkAccessManager;
	ThesaurusCache* cache;
	QString requestedWord;
};


class BigHugeThesaurus : public Thesaurus
{
public:
	BigHugeThesaurus(QObject* parent = 0);

protected:
	virtual QUrl makeUrl(const QString& word) const;
	virtual void parse(QNetworkReply* reply);
};

#endif // THESAURUS_H
