#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include <QString>

// ensures single instance across the Internet
// detects the existence of a lock file in the same directory upon starting
// deletes the lock when closing
// relies on online sync tools, such as dropbox, to work across the Internet
class SingleInstance
{
public:
    SingleInstance(const QString& appName);
    ~SingleInstance();

    bool run() const;   // if not locked, return true and create a lock

private:
    QString _lockName;
};

#endif // SINGLEINSTANCE_H
