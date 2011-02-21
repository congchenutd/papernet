#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include "ui_MainWindow.h"
#include "PaperModel.h"

class PagePapers;
class PageSnippets;

class QDataWidgetMapper;
struct ImportResult;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent*);

private slots:
	void onOptions();
	void onAbout();
	void delOldBackup();
	void backup(const QString& name = QString());

protected:
	virtual void resizeEvent(QResizeEvent*);

private slots:
	void onCurrentRowPapersChanged(const QModelIndex& idx);
	void onAddPaper();
	void onEditPaper();
	void onDelPaper();
	void onImport();
	void onSearch(const QString& target);
	void onSubmitPaper();
	void onClicked(const QModelIndex& idx);
	void onShowRelated();
	void onShowCoauthored();
	void onAddSnippet();
	void onResetPapers();

	void onCurrentRowAllTagsChanged();
	void onAddTag();
	void onDelTag();
	void onEditTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();

	void onCurrentRowTagsChanged();
	void onFilter(bool enabled);

private:
	int getPaperID (int row) const;
	int getTagID   (int row) const;
	int getAllTagID(int row) const;
	void selectID(int id);
	int idToRow(int id) const;
	void updateTags();
	void filterPapers();
	void resetAllTags();
	bool isFiltered() const;
	void hideRelated();
	void hideCoauthor();
	void mergeRecord(int row, const ImportResult& record);
	void insertRecord(const ImportResult& record);

public:
	enum {PAPER_ID, PAPER_READ, PAPER_TAGGED, PAPER_ATTACHED, PAPER_TITLE, 
		PAPER_AUTHORS, PAPER_YEAR, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_NOTE, 
		PAPER_PROXIMITY, PAPER_COAUTHOR, PAPER_ADDEDTIME};
	enum {TAG_ID, TAG_NAME};

private:
	Ui::MainWindowClass ui;

	PaperModel modelPapers;
	QSqlTableModel modelAllTags;
	QSqlQueryModel modelTags;
	int currentRowPapers;
	int currentRowTags;
	int currentPaperID;
	QDataWidgetMapper* mapper;
};

#endif // MAINWINDOW_H
