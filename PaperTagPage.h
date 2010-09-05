#ifndef PAPERTAGPAGE_H
#define PAPERTAGPAGE_H

#include <QWidget>
#include <QtGui>
#include <QtSql>
#include "ui_PaperTagPage.h"

class PaperTagPage : public QWidget
{
	Q_OBJECT

	typedef enum {ByPaper, ByTag} Mode;

public:
	PaperTagPage(QWidget *parent = 0);

private slots:
	void onByPaper();
	void onByTag();

private:
	void resetViews();

private:
	Ui::PaperTagPageClass ui;
	enum {PAPER_ID, PAPER_TITLE, PAPER_AUTHORS, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_PDF};
	enum {TAG_ID, TAG_NAME};

	QSqlTableModel tableModelPaper;
	QSqlTableModel tableModelTag;
	QSqlQueryModel queryModelPaper;
	QSqlQueryModel queryModelTag;
	QSqlQueryModel modelRelatedPapers;
	QSqlQueryModel modelRelatedTags;
	Mode mode;
};

#endif // PAPERTAGPAGE_H
