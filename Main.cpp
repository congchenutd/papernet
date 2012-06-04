#include "MainWindow.h"
#include "Common.h"

extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

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

	return app.exec();
}

// TODO: store spec in XML
// TODO: show required fields for each ref type
