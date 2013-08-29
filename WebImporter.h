#ifndef WEBIMPORTER_H
#define WEBIMPORTER_H

#include "Reference.h"
#include <QString>

class QUrl;

// import reference from a web page: bibtex and pdf
class WebImporter
{
public:
    bool parse(const QUrl& url);
    Reference getReference()   const;
    QString   getTempPDFPath() const;

public:
    static WebImporter* getInstance();

private:
    WebImporter() {}
    WebImporter(const WebImporter&) {}
    WebImporter& operator=(const WebImporter&) { return *this; }
    ~WebImporter() {}

private:
    static WebImporter* _instance;
    Reference           _ref;
    QString             _pdfPath;
};


#endif // WEBIMPORTER_H
