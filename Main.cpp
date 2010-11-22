#include "MainWindow.h"
#include "Common.h"
#include <QDesktopServices>
#include <QUrl>
#include <QtXml>

extern QString dbName;
extern QString attachmentDir;
extern QString emptyDir;

int main(int argc, char *argv[])
{
	QDomDocument doc("mydocument");
	QFile file("endnote.xml");
	if (!file.open(QIODevice::ReadOnly))
		return 1;
	if (!doc.setContent(&file)) {
		file.close();
		return 1;
	}
	file.close();

	QDomNodeList nodes = doc.elementsByTagName("RECORD");
	for(int i=0; i<nodes.count(); ++i)
	{
		QDomNode node = nodes.at(i);
		QDomNodeList records = node.toElement().elementsByTagName("AUTHOR");
		for(int j=0; j<records.count(); ++j)
		{
			QDomNode record = records.item(j);
			QString test = record.toElement().text();
		}
	}


	QApplication app(argc, argv);

	dbName = "PaperNet.db";
    attachmentDir = "./Attachments/";
	emptyDir = attachmentDir + "Empty";

    QDir::current().mkdir(attachmentDir);

	if(!openDB(dbName))
		return 1;
	createTables();

	MainWindow wnd;
	wnd.showMaximized();

	return app.exec();
}
