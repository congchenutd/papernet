#include "MainWindow.h"
#include "OptionDlg.h"
#include "Common.h"
#include <QMessageBox>
#include <QDate>
#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	ui.actionPapers->setChecked(true);

	QActionGroup* actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui.actionPapers);

	connect(ui.actionOptions, SIGNAL(triggered()), this, SLOT(onOptions()));
	connect(ui.actionAbout,   SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionPapers,  SIGNAL(triggered()), this, SLOT(onPapers()));

	// load settings
	qApp->setFont(MySetting<UserSetting>::getInstance()->getFont());
}

void MainWindow::onOptions()
{
	OptionDlg dlg(this);
	dlg.exec();
}

void MainWindow::onAbout()
{
	QMessageBox::about(this, "About", 
		tr("<h3><b>PaperNet</b></h3>"
		"<p>Build 2010.10.17</p>"
		"<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>"));
}

void MainWindow::closeEvent(QCloseEvent*)
{
	UserSetting* setting = MySetting<UserSetting>::getInstance();
	if(setting->getBackupDays() > 0)
	{
		delOldBackup();
		backup();
	}
	setting->destroySettingManager();
}

void MainWindow::delOldBackup()
{
	const QDate today = QDate::currentDate();
	const int   days  = MySetting<UserSetting>::getInstance()->getBackupDays();
	const QFileInfoList fileInfos = 
		QDir("Backup").entryInfoList(QStringList() << "*.db", QDir::Files);
	foreach(QFileInfo fileInfo, fileInfos)
		if(QDate::fromString(fileInfo.baseName(), Qt::ISODate).daysTo(today) > days)
			QFile::remove(fileInfo.filePath());
}

void MainWindow::backup(const QString& name)
{
	QDir::current().mkdir("Backup");
	QString backupFileName = name.isEmpty() 
		? "./Backup/" + QDate::currentDate().toString(Qt::ISODate) + ".db" 
		: name;

	if(QFile::exists(backupFileName))
		QFile::remove(backupFileName);

	extern QString dbName;
	QFile file(dbName);
	file.copy(backupFileName);
	file.close();
}

void MainWindow::onPapers() {
	ui.stackedWidget->setCurrentIndex(0);
}