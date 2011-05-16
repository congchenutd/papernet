#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"

class PagePapers;
class PageQuotes;
class PageDictionary;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	static MainWindow* getInstance();
	void jumpToPaper  (const QString& title);
	void jumpToSnippet(int snippetID);

protected:
	virtual void closeEvent(QCloseEvent*);

private slots:
	void onOptions();
	void onAbout();
	void delOldBackup();
	void backup(const QString& name = QString());
	void onPapers();
	void onQuotes();
	void onDictionary();

private:
	Ui::MainWindowClass ui;
	
	static MainWindow* instance;
	PagePapers*     pagePapers;
	PageQuotes*     pageQuotes;
	PageDictionary* pageDictionary;
};

#endif // MAINWINDOW_H
