#ifndef PAGESNIPPETS_H
#define PAGESNIPPETS_H

#include <QWidget>
#include <QSqlTableModel>
#include "ui_PageQuotes.h"

class PageQuotes : public QWidget
{
	Q_OBJECT

public:
	PageQuotes(QWidget *parent = 0);
	void jumpToSnippet(int snippetID);

private slots:
	void onAdd();
	void onDel();	
	void onCurrentRowChanged();
	void onSearch(const QString& target);
	void onEdit();
	void onAccepted();

signals:
	void tableValid(bool);

private:
	void resetSnippets();
	int getID(int row) const;

private:
	Ui::PageQuotesClass ui;

	QSqlTableModel model;
	int currentRow;
};

#endif // PAGESNIPPETS_H
