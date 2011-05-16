#include "MainWindow.h"
#include "OptionDlg.h"
#include "Common.h"
#include <QMessageBox>
#include <QDate>
#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	instance = this;

	ui.setupUi(this);
	onPapers();     // paper page by default

	pagePapers     = static_cast<PagePapers*>    (ui.stackedWidget->widget(0));
	pageQuotes     = static_cast<PageQuotes*>    (ui.stackedWidget->widget(1));
	pageDictionary = static_cast<PageDictionary*>(ui.stackedWidget->widget(2));

	QActionGroup* actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui.actionPapers);
	actionGroup->addAction(ui.actionQuotes);
	actionGroup->addAction(ui.actionDictionary);

	connect(ui.actionOptions,     SIGNAL(triggered()), this, SLOT(onOptions()));
	connect(ui.actionAbout,       SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionPapers,      SIGNAL(triggered()), this, SLOT(onPapers()));
	connect(ui.actionQuotes,      SIGNAL(triggered()), this, SLOT(onQuotes()));
	connect(ui.actionDictionary,  SIGNAL(triggered()), this, SLOT(onDictionary()));
	connect(ui.actionAboutQt,     SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionImportPaper, SIGNAL(triggered()), pagePapers, SLOT(onImport()));
	connect(ui.actionAddPaper,    SIGNAL(triggered()), pagePapers, SLOT(onAddPaper()));
	connect(ui.actionDelPaper,    SIGNAL(triggered()), pagePapers, SLOT(onDelPaper()));
	connect(ui.actionAddQuote,    SIGNAL(triggered()), pageQuotes, SLOT(onAdd()));
	connect(ui.actionDelQuote,    SIGNAL(triggered()), pageQuotes, SLOT(onDel()));
	connect(ui.actionAddPhrase,   SIGNAL(triggered()), pageDictionary, SLOT(onAdd()));
	connect(ui.actionDelPhrase,   SIGNAL(triggered()), pageDictionary, SLOT(onDel()));

	connect(ui.toolBarSearch, SIGNAL(search(QString)),         pagePapers, SLOT(onSearch(QString)));
    connect(ui.toolBarSearch, SIGNAL(search(QString)),         pageQuotes, SLOT(onSearch(QString)));
	connect(ui.toolBarSearch, SIGNAL(fullTextSearch(QString)), pagePapers, SLOT(onFullTextSearch(QString)));
	
	connect(pagePapers,     SIGNAL(tableValid(bool)), ui.actionDelPaper,  SLOT(setEnabled(bool)));
	connect(pageQuotes,     SIGNAL(tableValid(bool)), ui.actionDelQuote,  SLOT(setEnabled(bool)));
	connect(pageDictionary, SIGNAL(tableValid(bool)), ui.actionDelPhrase, SLOT(setEnabled(bool)));

	// load settings
	qApp->setFont(MySetting<UserSetting>::getInstance()->getFont());
}

void MainWindow::onOptions()
{
	OptionDlg dlg(this);
	dlg.exec();           // dlg will save the settings by itself
}

void MainWindow::onAbout()
{
	QMessageBox::about(this, "About", 
		tr("<h3><b>PaperNet: A Better Paper Manager</b></h3>"
		"<p>Build 2011.5.13</p>"
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
	pagePapers->saveSectionSizes();   // save the settings before the dtr
	pagePapers->saveSplitterSizes();
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
	ui.actionPapers->setChecked(true);
	ui.stackedWidget->setCurrentIndex(0);
    ui.toolBarSearch->onClear();
	ui.actionImportPaper->setVisible(true);
	ui.actionAddPaper->setVisible(true);
	ui.actionDelPaper->setVisible(true);
	ui.actionAddQuote->setVisible(false);
	ui.actionDelQuote->setVisible(false);
	ui.actionAddPhrase->setVisible(false);
	ui.actionDelPhrase->setVisible(false);
}

void MainWindow::onQuotes() 
{
	ui.actionQuotes->setChecked(true);
	ui.stackedWidget->setCurrentIndex(1);
    ui.toolBarSearch->onClear();
	ui.actionImportPaper->setVisible(false);
	ui.actionAddPaper->setVisible(false);
	ui.actionDelPaper->setVisible(false);
	ui.actionAddQuote->setVisible(true);
	ui.actionDelQuote->setVisible(true);
	ui.actionAddPhrase->setVisible(false);
	ui.actionDelPhrase->setVisible(false);
}

void MainWindow::onDictionary()
{
	ui.actionDictionary->setChecked(true);
	ui.stackedWidget->setCurrentIndex(2);
	ui.toolBarSearch->onClear();
	ui.actionImportPaper->setVisible(false);
	ui.actionAddPaper->setVisible(false);
	ui.actionDelPaper->setVisible(false);
	ui.actionAddQuote->setVisible(false);
	ui.actionDelQuote->setVisible(false);
	ui.actionAddPhrase->setVisible(true);
	ui.actionDelPhrase->setVisible(true);
}

void MainWindow::jumpToPaper(const QString& title)
{
	onPapers();
	pagePapers->jumpToPaper(title);
}

void MainWindow::jumpToSnippet(int snippetID)
{
	onQuotes();
	pageQuotes->jumpToSnippet(snippetID);
}

MainWindow* MainWindow::getInstance() {
	return instance;
}

MainWindow* MainWindow::instance = 0;
