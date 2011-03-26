#include "MainWindow.h"
#include "Common.h"
#include "OptionDlg.h"

extern QString userName;
extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;

void overwritingCopy(const QString& src, const QString& dest)
{
	QFile::remove(dest);
	QFile::copy(src, dest);
}

void loadDB()
{
	QString temp = MySetting<UserSetting>::getInstance()->getTempLocation();
	if(temp == ".")
		return;
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

    QDir::current().mkdir(attachmentDir);

	if(!openDB(dbName))
		return 1;
	createTables();

	MainWindow wnd;
	wnd.showMaximized();

//	makeFullTextFiles();

	int result = app.exec();
	saveDB();
	return result;
}