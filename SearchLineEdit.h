#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "ButtonLineEdit.h"

class QLabel;
class QPushButton;

class SearchLineEdit : public ButtonLineEdit
{
    Q_OBJECT

public:
    SearchLineEdit(QWidget* parent = 0);
    void setLabel(QLabel* label);
    void setClearButton (ClearButton*   button);
    void setSearchButton(PictureButton* button);
    void setShowSearchButton(bool show);

private slots:
    void onSearch();   // search button clicked
    void onFocus();

signals:
    void filter(const QString& target);
    void search(const QString& target);    // search button clicked

private:
    QLabel*        _label;
    ClearButton*   _btClear;
    PictureButton* _btSearch;
    QPushButton*   _btFocus;   // for grabbing focus for the lineedit
};

#endif // SEARCHLINEEDIT_H
