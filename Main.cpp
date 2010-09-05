#include "MainWindow.h"
#include "Common.h"
#include <QtGui>

QString dbName;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	dbName = "PaperNet.db";
	if(!openDB(dbName))
		return 1;
	createTables();

	MainWindow wnd;
	wnd.showMaximized();
	return app.exec();
}
