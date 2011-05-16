#ifndef PAGEDICTIONARY_H
#define PAGEDICTIONARY_H

#include <QWidget>
#include "ui_PageDictionary.h"
#include <QSqlTableModel>

class PageDictionary : public QWidget
{
	Q_OBJECT

public:
	PageDictionary(QWidget *parent = 0);

private slots:
	void onAdd();
	void onDel();
	void onEdit();
	void onCurrentRowChanged();

	void onAddTag();
	void onAddTagToPhrase();
	void onDelTagFromPhrase();
	void onFilterPhrases();

signals:
	void tableValid(bool);

private:
	Ui::PageDictionary ui;
	QSqlTableModel model;
	int currentRow;
};

#endif // PAGEDICTIONARY_H
