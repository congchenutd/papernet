#include "MainWindow.h"
#include "OptionDlg.h"
#include "Common.h"
#include "Navigator.h"
#include <QMessageBox>
#include <QDate>
#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	instance = this;
	currentPage = 0;
	navigator = Navigator::getInstance();

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
	connect(ui.actionAdd, SIGNAL(triggered()), this, SLOT(onAdd()));
	connect(ui.actionDel, SIGNAL(triggered()), this, SLOT(onDel()));
	connect(ui.actionBackward, SIGNAL(triggered()), this, SLOT(onBackward()));
	connect(ui.actionForward,  SIGNAL(triggered()), this, SLOT(onForward()));
	connect(ui.toolBarSearch, SIGNAL(search(QString)), this, SLOT(onSearch(QString)));
	connect(ui.toolBarSearch, SIGNAL(fullTextSearch(QString)), pagePapers, SLOT(onFullTextSearch(QString)));

	connect(pagePapers,     SIGNAL(tableValid(bool)), this, SLOT(onTableInvalid(bool)));
	connect(pageQuotes,     SIGNAL(tableValid(bool)), this, SLOT(onTableInvalid(bool)));
	connect(pageDictionary, SIGNAL(tableValid(bool)), this, SLOT(onTableInvalid(bool)));

	connect(navigator, SIGNAL(historyValid(bool)), ui.actionBackward, SLOT(setEnabled(bool)));
	connect(navigator, SIGNAL(futureValid (bool)), ui.actionForward,  SLOT(setEnabled(bool)));

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
		"<p>Build 2011.10.6</p>"
		"<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>"));
}

void MainWindow::closeEvent(QCloseEvent*)
{
	UserSetting* setting = UserSetting::getInstance();
	if(setting->getBackupDays() > 0)
	{
		delOldBackup();
		backup();
	}
	pagePapers->saveGeometry();   // save the settings before the dtr
	pageDictionary->saveGeometry();
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
	currentPage = ui.pagePapers;
	ui.actionPapers->setChecked(true);
	ui.stackedWidget->setCurrentIndex(0);
	ui.toolBarSearch->onClear();
	ui.actionImportPaper->setVisible(true);
}

void MainWindow::onQuotes()
{
	currentPage = ui.pageQuotes;
	ui.actionQuotes->setChecked(true);
	ui.stackedWidget->setCurrentIndex(1);
	ui.toolBarSearch->onClear();
	ui.actionImportPaper->setVisible(false);
}

void MainWindow::onDictionary()
{
	currentPage = ui.pageDictionary;
	ui.actionDictionary->setChecked(true);
	ui.stackedWidget->setCurrentIndex(2);
	ui.toolBarSearch->onClear();
	ui.actionImportPaper->setVisible(false);
}

void MainWindow::jumpToPaper(const QString& title)
{
	onPapers();
	pagePapers->jumpToID(::getPaperID(title));
}

void MainWindow::jumpToQuote(int quoteID)
{
	onQuotes();
	pageQuotes->jumpToID(quoteID);
}

MainWindow* MainWindow::getInstance() {
	return instance;
}

void MainWindow::onAdd() {
	currentPage->add();
}
void MainWindow::onDel() {
	currentPage->del();
}
void MainWindow::onSearch(const QString& target) {
	currentPage->search(target);
}
void MainWindow::onForward() {
	navigateTo(navigator->forward());
}
void MainWindow::onBackward() {
	navigateTo(navigator->backward());
}

void MainWindow::onTableInvalid(bool valid) {
	//if(Page* page = dynamic_cast<Page*>(sender()))
	//	if(currentPage == page)
	//		ui.actionDel->setEnabled(valid);
}

void MainWindow::navigateTo(const FootStep& footStep)
{
	if(footStep.page == 0)
		return;

	// switch to corresponding page
	currentPage = footStep.page;
	if(currentPage == ui.pagePapers)
		onPapers();
	else if(currentPage == ui.pageQuotes)
		onQuotes();
	else
		onDictionary();

	currentPage->jumpToID(footStep.id);   // jump to row
}

MainWindow* MainWindow::instance = 0;
