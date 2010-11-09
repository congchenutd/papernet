#ifndef PAGESNIPPETS_H
#define PAGESNIPPETS_H

#include <QWidget>
#include <QSqlTableModel>
#include "ui_PageSnippets.h"

class PageSnippets : public QWidget
{
	Q_OBJECT

public:
	PageSnippets(QWidget *parent = 0);

private slots:
	void onCurrentRowChanged();
	void onSearch(const QString& target);
	void onAdd();
	void onDel();
	void onEdit();

private:
	void resetSnippets();
	int getID(int row) const;

private:
	Ui::PageSnippetsClass ui;
	enum {SNIPPET_ID, SNIPPET_TITLE, SNIPPET_SNIPPET};

	QSqlTableModel model;
	int currentRow;
};

#endif // PAGESNIPPETS_H
