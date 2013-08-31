#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "ButtonLineEdit.h"

class SearchLineEdit : public ButtonLineEdit
{
    Q_OBJECT

public:
    SearchLineEdit(const QPixmap& pxLabel, const QPixmap& pxSearch,
                   QWidget* parent = 0);
    void setShowFullTextSearch(bool show);

private slots:
    void onFullText();
    void onFocus();

signals:
    void filter        (const QString& target);
    void fullTextSearch(const QString& target);

private:
    PictureButton* _btFullText;
};

#endif // SEARCHLINEEDIT_H
