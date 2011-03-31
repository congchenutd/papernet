#include "MainWindow.h"
#include "Common.h"
#include "OptionDlg.h"
#include <QDesktopServices>
#include <QUrl>

extern QString userName;
extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;
extern QString pdfDir;

void overwritingCopy(const QString& src, const QString& dest)
{
    QFile::remove(dest);
    QFile::copy(src, dest);
}

void loadDB()
{
    QString temp = MySetting<UserSetting>::getInstance()->getTempLocation();
    if(temp == ".")
    {
        dbName = "PaperNet.db";
        return;
    }
    dbName = temp + "/PaperNet.db";
    overwritingCopy("PaperNet.db", dbName);   // make a working copy, in case of conflict
}

void saveDB()
{
    UserSetting* setting = MySetting<UserSetting>::getInstance();
    QString temp = setting->getTempLocation();
    if(temp == ".")
        return;
    overwritingCopy(dbName, "PaperNet.db");   // save the working copy
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    loadDB();
    attachmentDir = "./Attachments/";
    emptyDir = attachmentDir + "Empty";
    pdfDir = attachmentDir + "PDF";

    QDir::current().mkdir(attachmentDir);
    QDir::current().mkdir(pdfDir);

    if(!openDB(dbName))
        return 1;
    createTables();

    MainWindow wnd;
    wnd.showMaximized();

    int result = app.exec();
    saveDB();
    return result;
}
