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
    PageDictionary(QWidget* parent = 0);
    void saveGeometry();    // called by mainwindow, closeEvent() only works for top window

    void addRecord();
    void delRecord();
    void editRecord();
    void search(const QString& target);
    void jumpToID(int id);
    void jumpToCurrent() { jumpToID(_currentPhraseID); }
    void reset() { onResetPhrases(); }

private slots:
    void onSelectionChanged(const QItemSelection& selected);
	void onEdit();
	void onClicked(const QModelIndex& idx);

	void onAddTag();
    void onAddTagsToPhrases();
    void onRemoveTagsFromPhrases();
    void onFilterByTags(bool AND = false);
	void onTagDoubleClicked(const QString& label);

    void onResetPhrases();
    void onRelatedDoubleClicked(int phraseID);

signals:
    void selectionValid(bool);

private:
    int  rowToID(int row) const;
    void updateTags(int phraseID, const QStringList& tags); // reset phraseID's tags
    void highLightTags();                                   // highlight tags of current phrase
	void loadGeometry();

private:
	Ui::PageDictionary ui;
    QSqlTableModel _model;
    int            _currentRow;
    int            _currentPhraseID;
};

#endif // PAGEDICTIONARY_H
