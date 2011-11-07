#include "Thesaurus.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QStringList>
#include <QTextStream>
#include <QNetworkReply>

Thesaurus::Thesaurus(QObject *parent) : QObject(parent)
{
	networkAccessManager = new QNetworkAccessManager(this);
	connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(parse(QNetworkReply*)));
}

void Thesaurus::request(const QString& word) const {
	networkAccessManager->get(QNetworkRequest(makeUrl(word)));
}

//////////////////////////////////////////////////////////////////////////
BigHugeThesaurus::BigHugeThesaurus(QObject* parent) : Thesaurus(parent) {}

QUrl BigHugeThesaurus::makeUrl(const QString& word) const {
	return QUrl(tr("http://words.bighugelabs.com/api/2/4c0966eb0e369b282100a3c599c66c46/%1/")
		.arg(word));
}

void BigHugeThesaurus::parse(QNetworkReply* reply)
{
	QStringList results;
	QTextStream is(reply);
	while(!is.atEnd()) {
		QStringList sections = is.readLine().split("|");  // a line contains 3 sections
		if(sections.size() == 3)
			results << sections[2];                       // the last section is the word
	}
	emit response(results);
}
