#include "MainWindow.h"
#include "Common.h"
#include <QFileSystemModel>
#include <QListView>

QString dbName;

extern QString attachmentDir;
extern QString emptyDir;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	dbName = "PaperNet.db";
	attachmentDir = ".\\Attachments\\";
	emptyDir = attachmentDir + "Empty";

	if(!openDB(dbName))
		return 1;
	createTables();

	MainWindow wnd;
	wnd.showMaximized();

	//QFileSystemModel model;
	//model.setRootPath("F:\\Program\\PaperNet\\Attachments");
	//model.setNameFilters(QStringList() << "a - *");
	//model.setNameFilterDisables(false);
	//QListView view;
	//view.setViewMode(QListView::IconMode);
	//view.setModel(&model);
	//view.setRootIndex(model.index("F:\\Program\\PaperNet\\Attachments"));
	//view.show();

	return app.exec();
}
