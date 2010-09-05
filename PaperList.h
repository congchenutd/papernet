#ifndef PAPERLIST_H
#define PAPERLIST_H

#include <QWidget>
#include <QtSql>
#include "ui_PaperList.h"

class PaperList : public QWidget
{
	Q_OBJECT

public:
	PaperList(QWidget *parent = 0);

private slots:
	void onAddPaper();
	void onDelPaper();
	void onSubmitPaper();
	void onAddTag();
	void onDelTag();
	void onCurrentRowChanged(const QModelIndex& idx);
	void onImport();
	void onSetPDF();
	void onReadPDF();
	void onSearch(const QString& target);
	void onCancelSearch();
	void onEnableDelTags(const QModelIndex& idx);

private:
	int getCurrentPaperID() const;
	void updateTags();
	void updateRelatedPapers();
	void import(const QString& fileName,    const QString& firstHead,
				const QString& titleHead,   const QString& authorHead, 
				const QString& journalHead, const QString& delimiter,
				const QString& abstractHead = QString());
	QString trimHead(const QString& line, const QString& delimiter) const;
	QString makePDFFileName(const QString& title);
	void resetModel();

public:
	enum {PAPER_ID, PAPER_TITLE, PAPER_AUTHORS, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_PDF};

private:
	Ui::PaperListClass ui;
	int currentRow;
	QSqlTableModel modelPapers;
	QSqlQueryModel modelTags;
	QSqlQueryModel modelRelatedPapers;
};

#endif // PAPERLIST_H