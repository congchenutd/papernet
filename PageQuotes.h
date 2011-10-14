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
	virtual void reset() { resetQuotes(); }

private slots:
	void onCurrentRowChanged();
	void onEdit();
	void onAccepted();   // for the dlg to submit
	void onClicked(const QModelIndex& idx);

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
