#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
	virtual void closeEvent(QCloseEvent*);

private slots:
	void onOptions();
	void onAbout();
	void delOldBackup();
	void backup(const QString& name = QString());
	void onPapers();

private:
	Ui::MainWindowClass ui;
};

#endif // MAINWINDOW_H
