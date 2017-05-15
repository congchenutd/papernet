#include "SingleInstance.h"
#include <QFile>

SingleInstance::SingleInstance(const QString& appName)
    : _lockName(appName + ".lock") {}

void SingleInstance::close() {
    QFile::remove(_lockName);
}

bool SingleInstance::run() const {
    return QFile::exists(_lockName) ? false
                                    : QFile(_lockName).open(QFile::WriteOnly);
}
