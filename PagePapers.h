#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include "ui_PagePapers.h"

class QDataWidgetMapper;

class PagePapers : public QWidget
{
	Q_OBJECT

public:
	PagePapers(QWidget *parent = 0);
	~PagePapers();

private slots:
	void onCurrentRowPapersChanged(const QModelIndex& idx);
	void onAddPaper();
	void onDelPaper();
	void onImport();
	void onSearch(const QString& target);
	void onCancelSearch();
	void onSetPDF();
	void onReadPDF();

	void onCurrentRowAllTagsChanged();
	void onAddTag();
	void onDelTag();
	void onEditTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();

	void onCurrentRowTagsChanged();

private:
	int getPaperID(int row) const;
	int getTagID  (int row) const;
	int getCurrentPaperID() const;
	void selectID(int id);
	void import(const QString& fileName, const QString& firstHead, 
				const QString& titleHead, const QString& authorHead, 
				const QStringList& journalHeads, const QString& delimiter, 
				const QString& abstractHead = "NO_SUCH_HEAD");
	QString trimHead(const QString& line, const QString& delimiter) const;
	void submitPaper();
	int idToRow(int id) const;
	QString getCurrentPDFPath() const;
	QString makePDFFileName(const QString& title) const;
	void updateTags();
	void updatePapers();

private:
	enum {PAPER_ID, PAPER_TITLE, PAPER_AUTHORS, PAPER_JOURNAL, 
		  PAPER_ABSTRACT, PAPER_NOTE, PAPER_PDF};
	enum {TAG_ID, TAG_NAME};

	Ui::PagePapersClass ui;

	QSqlTableModel modelPapers;
	QSqlTableModel modelAllTags;
	QSqlQueryModel modelTags;
	int currentRowPapers;
	int currentRowTags;
	QDataWidgetMapper* mapper;
};

#endif // PAGEPAPERS_H
