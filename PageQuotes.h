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
	
    void addRecord();
    void delRecord();
    void editRecord();
    void search(const QString& target);
    void jumpToID(int id);
    void jumpToCurrent() { reset(); }   // currentID not supported yet
    void reset();

private slots:
    void onSelectionChanged(const QItemSelection& selected);
	void onEdit();
	void onClicked(const QModelIndex& idx);

signals:
    void selectionValid(bool);

private:
	int rowToID(int row) const;

private:
	Ui::PageQuotesClass ui;

	QSqlTableModel _model;
	int            _currentRow;
};

#endif // PAGEQUOTES_H
