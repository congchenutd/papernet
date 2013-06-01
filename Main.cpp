#include "MainWindow.h"
#include "Common.h"
#include "SingleInstance.h"
#include <QApplication>
#include <QMessageBox>

extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // the database cannot be accessed by multiple instances
    SingleInstance singleInstance("PaperNet");
    if(!singleInstance.run())
    {
        QMessageBox::critical(0, QObject::tr("Error"),
            QObject::tr("Another instance is already running on this or a synchronized computer."
                        "Close the existing instance before launching a new instance."));
        return 1;
    }

    dbName        = "PaperNet.db";
    attachmentDir = "./Attachments/";
    emptyDir      = attachmentDir + "Empty";
    QDir::current().mkdir(attachmentDir);
    QDir::current().mkdir(emptyDir);

    if(!openDB(dbName))
        return 1;
    createTables();

    MainWindow wnd;
    wnd.showMaximized();

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
