#ifndef PAGEDICTIONARY_H
#define PAGEDICTIONARY_H

#include <QSqlTableModel>
#include "ui_PageDictionary.h"
#include "Page.h"
#include "DictionaryModel.h"

class PageDictionary : public Page
{
	Q_OBJECT

public:
	PageDictionary(QWidget *parent = 0);
	void saveGeometry();

	virtual void add();
	virtual void del();
	virtual void search(const QString& target);
	virtual void jumpToID(int id);

private slots:
	void onEdit();
	void onCurrentRowChanged();
	void onClicked(const QModelIndex& idx);
	void onShowRelated();

	void onAddTag();
	void onAddTagToPhrase();
	void onDelTagFromPhrase();
	void onFilterPhrases();   // by tags
	void onResetPhrases();
	void onTagDoubleClicked(const QString& label);

signals:
	void tableValid(bool);

private:
	int getID(int row) const;
	void submit();
	void updateTags(const QStringList& tags);
	void highLightTags();
	void hideRelated();

private:
	Ui::PageDictionary ui;
	DictionaryModel model;
	int currentRow;
	int currentPhraseID;
};

#endif // PAGEDICTIONARY_H
