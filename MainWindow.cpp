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
    ui.toolBarMain->initSearchBar();
	ui.actionPapers->setChecked(true);
	onPapers();

	pagePapers   = static_cast<PagePapers*>  (ui.stackedWidget->widget(0));
	pageSnippets = static_cast<PageSnippets*>(ui.stackedWidget->widget(1));

	QActionGroup* actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui.actionPapers);
	actionGroup->addAction(ui.actionSnippets);

	connect(ui.actionOptions,     SIGNAL(triggered()), this, SLOT(onOptions()));
	connect(ui.actionAbout,       SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionPapers,      SIGNAL(triggered()), this, SLOT(onPapers()));
	connect(ui.actionSnippets,    SIGNAL(triggered()), this, SLOT(onSnippets()));
    connect(ui.toolBarMain, SIGNAL(search(QString)), pagePapers,   SLOT(onSearch(QString)));
    connect(ui.toolBarMain, SIGNAL(search(QString)), pageSnippets, SLOT(onSearch(QString)));
	connect(ui.actionImportPaper, SIGNAL(triggered()), pagePapers,   SLOT(onImport()));
	connect(ui.actionAddPaper,    SIGNAL(triggered()), pagePapers,   SLOT(onAddPaper()));
	connect(ui.actionDelPaper,    SIGNAL(triggered()), pagePapers,   SLOT(onDelPaper()));
	connect(ui.actionAddSnippet,  SIGNAL(triggered()), pageSnippets, SLOT(onAdd()));
	connect(ui.actionDelSnippet,  SIGNAL(triggered()), pageSnippets, SLOT(onDel()));
	connect(pagePapers,   SIGNAL(tableValid(bool)), ui.actionDelPaper,   SLOT(setEnabled(bool)));
	connect(pageSnippets, SIGNAL(tableValid(bool)), ui.actionDelSnippet, SLOT(setEnabled(bool)));

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
		tr("<h3><b>PaperNet: a better paper manager</b></h3>"
		"<p>Build 2011.2.21</p>"
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

void MainWindow::onPapers() 
{
	ui.stackedWidget->setCurrentIndex(0);
    ui.toolBarMain->onClear();
	ui.actionImportPaper->setVisible(true);
	ui.actionAddPaper->setVisible(true);
	ui.actionDelPaper->setVisible(true);
	ui.actionAddSnippet->setVisible(false);
	ui.actionDelSnippet->setVisible(false);
}

void MainWindow::onSnippets() 
{
	ui.stackedWidget->setCurrentIndex(1);
    ui.toolBarMain->onClear();
	ui.actionImportPaper->setVisible(false);
	ui.actionAddPaper->setVisible(false);
	ui.actionDelPaper->setVisible(false);
	ui.actionAddSnippet->setVisible(true);
	ui.actionDelSnippet->setVisible(true);
}

void MainWindow::jumpToPaper(const QString& title)
{
	ui.actionPapers->setChecked(true);
	onPapers();
	pagePapers->jumpToPaper(title);
}

void MainWindow::jumpToSnippet(int snippetID)
{
	ui.actionSnippets->setChecked(true);
	onSnippets();
	pageSnippets->jumpToSnippet(snippetID);
}

MainWindow& MainWindow::getInstance()
{
	static MainWindow instance;
	return instance;
}
