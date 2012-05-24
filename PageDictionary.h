#ifndef PAGEDICTIONARY_H
#define PAGEDICTIONARY_H

#include <QSqlTableModel>
#include "ui_PageDictionary.h"
#include "Page.h"
#include <QSqlTableModel>

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
	virtual void jumpToCurrent() { jumpToID(currentPhraseID); }
	virtual void reset() { onResetPhrases(); }

private slots:
	void onEdit();
	void onCurrentRowChanged();
	void onClicked(const QModelIndex& idx);

	void onAddTag();
	void onAddTagToPhrase();
	void onDelTagFromPhrase();
    void onFilterByTags(bool AND = false);
	void onResetPhrases();
	void onTagDoubleClicked(const QString& label);

    void onRelatedDoubleClicked(int phraseID);

signals:
	void tableValid(bool);

private:
    int rowToID(int row) const;
	void updateTags(const QStringList& tags);
	void highLightTags();
	void loadGeometry();

private:
	Ui::PageDictionary ui;
    QSqlTableModel model;
	int currentRow;
	int currentPhraseID;
};

#endif // PAGEDICTIONARY_H
