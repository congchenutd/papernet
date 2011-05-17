#ifndef PAGEQUOTES_H
#define PAGEQUOTES_H

#include <QWidget>
#include <QSqlTableModel>
#include "ui_PageQuotes.h"

class PageQuotes : public QWidget
{
	Q_OBJECT

public:
	PageQuotes(QWidget *parent = 0);
	void jumpToQuote(int quoteID);

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
	void resetQuotes();
	int getID(int row) const;

private:
	Ui::PageQuotesClass ui;

	QSqlTableModel model;
	int currentRow;
};

#endif // PAGEQUOTES_H
