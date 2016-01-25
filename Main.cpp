#include "MainWindow.h"
#include "Common.h"
#include "SingleInstance.h"
#include <QApplication>
#include <QHostInfo>
#include <QMessageBox>

extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;
extern QString backupDir;

// workaround for a bug on Mavericks
// Finder returns / as the working path of an app bundle
// but if the app is run from terminal, the path is correct
// This method calcluates the path of the bundle from the application's path
QString getCurrentPath()
{
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
    return dir.absolutePath();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef Q_OS_OSX
    QDir::setCurrent(getCurrentPath());
#endif

    // the database cannot be accessed by multiple instances
    SingleInstance singleInstance("PaperNet");
    if(!singleInstance.run())
    {
        QMessageBox::StandardButton response =
        QMessageBox::critical(0, QObject::tr("Multiple instances"),
            QObject::tr("Another instance is already running on this or a synchronized computer. "
                        "Close the existing instance before launching a new instance. "
                        "Continue launching this instance may cause synchronization errors. "
                        "Continue anyway?"),
            QMessageBox::Yes | QMessageBox::No);
        if (response == QMessageBox::No)
            return 1;
    }

    dbName        = "PaperNet.db";
    attachmentDir = "Attachments";
    emptyDir      = attachmentDir + "/Empty";
    backupDir     = "Backup";
    QDir::current().mkdir(attachmentDir);
    QDir::current().mkdir(emptyDir);
    QDir::current().mkdir(backupDir);

    if(!openDB(dbName))
        return 1;
    createTables();

    MainWindow wnd;
    wnd.showMaximized();

    // TODO: still necessary?
    if(argc > 1)
    {
        QStringList files;
        for(int i = 1; i < argc; ++i)
            files << QString(argv[i]);
        wnd.importRefFromFiles(files);
    }

    int result = app.exec();
    singleInstance.close();
    return result;
}
