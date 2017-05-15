#include "WebImporter.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

void WebImporter::parse(const QUrl& url)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this,     SLOT(onLoaded(QNetworkReply*)));

    manager->get(QNetworkRequest(url));
}

void WebImporter::onLoaded(QNetworkReply* reply)
{
    QString content = reply->readAll();
    qDebug() << content;
}

Reference WebImporter::getReference() const
{
    return Reference();
}

WebImporter* WebImporter::getInstance()
{
    if(_instance == 0)
        _instance = new WebImporter();
    return _instance;
}

WebImporter* WebImporter::_instance = 0;
