#include "MainWindow.h"
#include "Common.h"
#include <QResource>

extern QString userName;
extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;
extern QString pdfDir;
extern QString compileDate;

void overwritingCopy(const QString& src, const QString& dest)
{
	QFile::remove(dest);
	QFile::copy(src, dest);
}

void loadDB()
{
	QString temp = MySetting<UserSetting>::getInstance()->getTempLocation();
	if(temp == "." || temp.isEmpty())
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

void loadCompileDate()
{
	// this resource file will be generated after running CompileDate.bat
	QResource resource(":/MainWindow/CompileDate.txt");
	compileDate = (char*)resource.data();
	if(compileDate.isEmpty())
		compileDate = "Unknown";
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	loadDB();
	attachmentDir = "./Attachments/";
	emptyDir = attachmentDir + "Empty";
	pdfDir = "./PDF";
	loadCompileDate();

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
