#include "MainWindow.h"
#include "Common.h"

extern QString userName;
extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;
extern QString pdfDir;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	dbName        = "PaperNet.db";
	attachmentDir = "./Attachments/";
	emptyDir      = attachmentDir + "Empty";
	pdfDir        = "./PDF";

	QDir::current().mkdir(attachmentDir);
	QDir::current().mkdir(pdfDir);

	if(!openDB(dbName))
		return 1;
	createTables();

	MainWindow wnd;
	wnd.showMaximized();

	return app.exec();
}

// TODO: store spec in XML
// TODO: show required fields for each ref type
