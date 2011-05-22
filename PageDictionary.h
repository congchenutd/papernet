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
	void saveGeometry();

private:
	void highLightTags();

private slots:
	void onAdd();
	void onDel();
	void onEdit();
	void onCurrentRowChanged();

	void onAddTag();
	void onAddTagToPhrase();
	void onDelTagFromPhrase();
	void onFilterPhrases();
	void onResetPhrases();

signals:
	void tableValid(bool);

private:
	void selectID(int id);
	void submit();

private:
	Ui::PageDictionary ui;
	QSqlTableModel model;
	int currentRow;
	int currentPhraseID;
};

#endif // PAGEDICTIONARY_H
