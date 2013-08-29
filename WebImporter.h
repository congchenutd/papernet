#ifndef WEBIMPORTER_H
#define WEBIMPORTER_H

#include "Reference.h"
#include <QString>

class QUrl;
class QNetworkReply;

// import reference from a web page: bibtex and pdf
class WebImporter : public QObject
{
    Q_OBJECT

public:
    void parse(const QUrl& url);
    Reference getReference() const;

public:
    static WebImporter* getInstance();

private slots:
    void onLoaded(QNetworkReply*);

private:
    WebImporter() {}
    WebImporter(const WebImporter&) {}
    WebImporter& operator=(const WebImporter&) { return *this; }
    ~WebImporter() {}

private:
    static WebImporter* _instance;
};


#endif // WEBIMPORTER_H
