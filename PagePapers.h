#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QWidget>
#include <QSqlTableModel>
#include "ui_PagePapers.h"

class QDataWidgetMapper;

class PagePapers : public QWidget
{
	Q_OBJECT

public:
	PagePapers(QWidget *parent = 0);

private slots:
	void onCurrentRowPaperChanged(const QModelIndex& idx);
	void onAddPaper();
	void onDelPaper();

private:
	int getPaperID(int row) const;
	void selectID(int id);

private:
	enum {PAPER_ID, PAPER_TITLE, PAPER_AUTHORS, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_PDF};
	enum {TAG_ID, TAG_NAME};

	Ui::PagePapersClass ui;

	QSqlTableModel modelPapers;
	QSqlTableModel modelAllTags;
	int currentRowPapers;
	int currentRowTags;
	QDataWidgetMapper* mapper;
};

#endif // PAGEPAPERS_H
