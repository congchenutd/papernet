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

public slots:
	void onAdd();
	void onDel();	

private slots:
	void onCurrentRowChanged();
	void onSearch(const QString& target);
	void onEdit();

signals:
	void tableValid(bool);

private:
	void resetSnippets();
	int getID(int row) const;

private:
	Ui::PageSnippetsClass ui;

	QSqlTableModel model;
	int currentRow;
};

#endif // PAGESNIPPETS_H
