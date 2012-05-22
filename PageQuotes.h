#ifndef PAGEQUOTES_H
#define PAGEQUOTES_H

#include <QSqlTableModel>
#include "ui_PageQuotes.h"
#include "Page.h"

class PageQuotes : public Page
{
	Q_OBJECT

public:
	PageQuotes(QWidget *parent = 0);
	
	virtual void add();
	virtual void del();
	virtual void search(const QString& target);
	virtual void jumpToID(int id);
    virtual void jumpToCurrent() {}
    virtual void reset();
    virtual void enter() { reset(); }

private slots:
	void onCurrentRowChanged();
	void onEdit();
	void onClicked(const QModelIndex& idx);

signals:
	void tableValid(bool);

private:
	int getID(int row) const;
	void sortByTitle();

private:
	Ui::PageQuotesClass ui;

	QSqlTableModel model;
	int currentRow;
};

#endif // PAGEQUOTES_H
