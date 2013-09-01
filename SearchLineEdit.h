#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "EmbeddableLineEdit.h"

class QLabel;
class QPushButton;

class SearchLineEdit : public EmbeddableLineEdit
{
    Q_OBJECT

public:
    SearchLineEdit(QWidget* parent = 0);
    void setLabel(QLabel* label);
    void setClearButton (ClearButton*   button);
    void setSearchButton(PictureButton* button);
    void setFocusShortcut(const QKeySequence& keySequence);
    void setShowSearchButton(bool show);

private slots:
    void onSearch();   // search button clicked
    void onFocus();
    void onClear();

signals:
    void filter(const QString& target);
    void search(const QString& target);    // search button clicked

private:
    QLabel*        _label;
    ClearButton*   _btClear;
    PictureButton* _btSearch;
};

#endif // SEARCHLINEEDIT_H
