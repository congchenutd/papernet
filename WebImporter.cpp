#include "WebImporter.h"
#include <QUrl>

bool WebImporter::parse(const QUrl& url)
{
    return true;
}

Reference WebImporter::getReference() const {
    return _ref;
}

QString WebImporter::getTempPDFPath() const {
    return _pdfPath;
}

WebImporter* WebImporter::getInstance()
{
    if(_instance == 0)
        _instance = new WebImporter();
    return _instance;
}

WebImporter* WebImporter::_instance = 0;
