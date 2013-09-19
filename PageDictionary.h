#ifndef PAGEDICTIONARY_H
#define PAGEDICTIONARY_H

#include <QSqlTableModel>
#include "ui_PageDictionary.h"
#include "Page.h"

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
    void reset() { resetPhrases(); }

private slots:
    void onSelectionChanged(const QItemSelection& selected);
	void onEdit();
	void onClicked(const QModelIndex& idx);

	void onAddTag();
    void onAddTagsToPhrases();
    void onRemoveTagsFromPhrases();
    void onFilterByTags(bool AND = false);
	void onTagDoubleClicked(const QString& label);

    void onRelatedDoubleClicked(int phraseID);

signals:
    void selectionValid(bool);

private:
    void resetPhrases();
    int  rowToID(int row) const;
    void updateTags(int phraseID, const QStringList &tags);
	void loadGeometry();

private:
	Ui::PageDictionary ui;
    QSqlTableModel _model;
    int            _currentRow;
    int            _currentPhraseID;
};

#endif // PAGEDICTIONARY_H
