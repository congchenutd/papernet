#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"

class PagePapers;
class PageQuotes;
class PageDictionary;
class Navigator;
struct FootStep;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    typedef enum {PAGE_PAPERS, PAGE_QUOTES, PAGE_DICTIONARY} PageIndex;

public:
    MainWindow(QWidget* parent = 0);
    static MainWindow* getInstance();   // access for pages
    void jumpToPaper(int paperID);
	void jumpToQuote(int quoteID);
    void importRefFromFiles(const QStringList& filePaths);  // for external apps
    int getCurrentPageIndex() const;    // which page is currently at

protected:
	virtual void closeEvent(QCloseEvent*);

private slots:
	void onOptions();
	void onAbout();
	void onPapers();
	void onQuotes();
	void onDictionary();
	void onAdd();
	void onDel();
	void onSearch(const QString& target);
	void onClearSearch();
	void onForward();
	void onBackward();
    void onSelectionValid(bool valid);

private:
	void delOldBackup();
	void backup(const QString& name = QString());
	void navigateTo(const FootStep& footStep);

private:
	Ui::MainWindowClass ui;

    static MainWindow* _instance;
    PagePapers*     _pagePapers;
    PageQuotes*     _pageQuotes;
    PageDictionary* _pageDictionary;
    Page*           _currentPage;
    Navigator*      _navigator;
};

#endif // MAINWINDOW_H
