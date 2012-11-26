#include "MainWindow.h"
#include "OptionDlg.h"
#include "Common.h"
#include "Navigator.h"
#include <QMessageBox>
#include <QDate>
#include <QActionGroup>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      _currentPage(0),
      _navigator(Navigator::getInstance())
{
    _instance = this;
	ui.setupUi(this);

	_pagePapers     = static_cast<PagePapers*>    (ui.stackedWidget->widget(0));
	_pageQuotes     = static_cast<PageQuotes*>    (ui.stackedWidget->widget(1));
	_pageDictionary = static_cast<PageDictionary*>(ui.stackedWidget->widget(2));

    // action group makes actions mutual exclusive
	QActionGroup* actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui.actionPapers);
	actionGroup->addAction(ui.actionQuotes);
	actionGroup->addAction(ui.actionDictionary);

    connect(ui.actionAboutQt,    SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui.actionOptions,    SIGNAL(triggered()), this, SLOT(onOptions()));
    connect(ui.actionAbout,      SIGNAL(triggered()), this, SLOT(onAbout()));
    connect(ui.actionPapers,     SIGNAL(triggered()), this, SLOT(onPapers()));
    connect(ui.actionQuotes,     SIGNAL(triggered()), this, SLOT(onQuotes()));
    connect(ui.actionDictionary, SIGNAL(triggered()), this, SLOT(onDictionary()));
    connect(ui.actionAdd,        SIGNAL(triggered()), this, SLOT(onAdd()));
    connect(ui.actionDel,        SIGNAL(triggered()), this, SLOT(onDel()));
    connect(ui.actionBackward,   SIGNAL(triggered()), this, SLOT(onBackward()));
    connect(ui.actionForward,    SIGNAL(triggered()), this, SLOT(onForward()));
    connect(ui.toolBarSearch,    SIGNAL(search(QString)), this, SLOT(onSearch(QString)));
    connect(ui.toolBarSearch,    SIGNAL(clearSearch()),   this, SLOT(onClearSearch()));
    connect(ui.actionImportRef,  SIGNAL(triggered()),             _pagePapers, SLOT(onImport()));
    connect(ui.actionExportRef,  SIGNAL(triggered()),             _pagePapers, SLOT(onExport()));
    connect(ui.actionReadPDF,    SIGNAL(triggered()),             _pagePapers, SLOT(onReadPDF()));
    connect(ui.toolBarSearch,    SIGNAL(fullTextSearch(QString)), _pagePapers, SLOT(onFullTextSearch(QString)));

    connect(_pagePapers,     SIGNAL(selectionValid(bool)), this, SLOT(onSelectionValid(bool)));
    connect(_pageQuotes,     SIGNAL(selectionValid(bool)), this, SLOT(onSelectionValid(bool)));
    connect(_pageDictionary, SIGNAL(selectionValid(bool)), this, SLOT(onSelectionValid(bool)));

    connect(_pagePapers, SIGNAL(hasPDF(bool)),       ui.actionReadPDF,  SLOT(setEnabled(bool)));
    connect(_navigator,  SIGNAL(historyValid(bool)), ui.actionBackward, SLOT(setEnabled(bool)));
    connect(_navigator,  SIGNAL(futureValid (bool)), ui.actionForward,  SLOT(setEnabled(bool)));

	// load settings
	UserSetting* setting = UserSetting::getInstance();
	qApp->setFont(setting->getFont());

    onPapers();   // go to papers page by default
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
           "<p>Built on %1</p>"
           "<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>")
                       .arg(UserSetting::getInstance()->getCompileDate()));
}

void MainWindow::closeEvent(QCloseEvent*)
{
	UserSetting* setting = UserSetting::getInstance();
    if(setting->getBackupDays() > 0)  // 0 means do not backup
	{
		delOldBackup();
		backup();
	}
    _pagePapers    ->saveGeometry();   // save the settings before the dtr
	_pageDictionary->saveGeometry();
	setting->destroySettingManager();
}

void MainWindow::delOldBackup()
{
    QDate today      = QDate::currentDate();
    int   daysToKeep = UserSetting::getInstance()->getBackupDays();

    // get all the .db files
    QFileInfoList fileInfos =
		QDir("Backup").entryInfoList(QStringList() << "*.db", QDir::Files);

    // for each .db file, remove it if it's older than daysToKeep
	foreach(QFileInfo fileInfo, fileInfos)
        if(QDate::fromString(fileInfo.baseName(), Qt::ISODate).daysTo(today) > daysToKeep)
			QFile::remove(fileInfo.filePath());
}

void MainWindow::backup(const QString& name)
{
	QDir::current().mkdir("Backup");
    QString backupFileName = name.isEmpty()
            ? "./Backup/" + QDate::currentDate().toString(Qt::ISODate) + ".db"
            : name;

    if(QFile::exists(backupFileName))   // only keep on backup for each day
		QFile::remove(backupFileName);

	extern QString dbName;
	QFile file(dbName);
	file.copy(backupFileName);
	file.close();
}

void MainWindow::onPapers()
{
	ui.actionPapers->setChecked(true);
    ui.stackedWidget->setCurrentIndex(PAGE_PAPERS);
    ui.actionImportRef->setVisible(true);
    ui.actionExportRef->setVisible(true);
    ui.actionReadPDF  ->setVisible(true);
    _currentPage = ui.pagePapers;
    _currentPage->jumpToCurrent();
//    onSelectionValid(false);         // select row 0 by default
}

void MainWindow::onQuotes()
{
	ui.actionQuotes->setChecked(true);
    ui.stackedWidget->setCurrentIndex(PAGE_QUOTES);
    ui.actionImportRef->setVisible(false);
    ui.actionExportRef->setVisible(false);
    ui.actionReadPDF  ->setVisible(false);
    _currentPage = ui.pageQuotes;
    _currentPage->reset();          // quotes may be changd by paper page
    _currentPage->jumpToCurrent();
    onSelectionValid(false);
}

void MainWindow::onDictionary()
{
	ui.actionDictionary->setChecked(true);
    ui.stackedWidget->setCurrentIndex(PAGE_DICTIONARY);
    ui.actionImportRef->setVisible(false);
    ui.actionExportRef->setVisible(false);
    ui.actionReadPDF  ->setVisible(false);
    _currentPage = ui.pageDictionary;
	_currentPage->jumpToCurrent();
    onSelectionValid(false);
}

void MainWindow::jumpToPaper(int paperID)
{
	onPapers();
    _pagePapers->reset();                 // ensure the row is visible
    _pagePapers->jumpToID(paperID);
    _navigator->addFootStep(ui.pagePapers, paperID);
}

void MainWindow::jumpToQuote(int quoteID)
{
	onQuotes();
	_pageQuotes->reset();                 // ensure the row is visible
	_pageQuotes->jumpToID(quoteID);
    _navigator->addFootStep(ui.pageQuotes, quoteID);
}

void MainWindow::importRefFromFiles(const QStringList& filePaths)
{
    onPapers();    // switch to paper page
    static_cast<PagePapers*>(_currentPage)->importFromFiles(filePaths);
}

int MainWindow::getCurrentPageIndex() const {
    return ui.stackedWidget->currentIndex();
}

MainWindow* MainWindow::getInstance() {
	return _instance;
}

void MainWindow::onAdd() {
    _currentPage->addRecord();
}
void MainWindow::onDel() {
    _currentPage->delRecord();
}
void MainWindow::onSearch(const QString& target) {
	_currentPage->search(target);
}

void MainWindow::onClearSearch()
{
	_currentPage->reset();
    _currentPage->jumpToCurrent();
}

void MainWindow::onForward() {
	navigateTo(_navigator->forward());
}
void MainWindow::onBackward() {
    navigateTo(_navigator->backward());
}

void MainWindow::onSelectionValid(bool valid)
{
    ui.actionDel      ->setEnabled(valid);
    ui.actionExportRef->setEnabled(valid);
}

void MainWindow::navigateTo(const FootStep& footStep)
{
    if(footStep._page == 0)     // invalid
		return;

	// switch to corresponding page
	_currentPage = footStep._page;
	if(_currentPage == ui.pagePapers)
		onPapers();
	else if(_currentPage == ui.pageQuotes)
		onQuotes();
	else
		onDictionary();

	_currentPage->reset();                 // ensure the row is visible
    _currentPage->jumpToID(footStep._id);  // jump to row
}

MainWindow* MainWindow::_instance = 0;

