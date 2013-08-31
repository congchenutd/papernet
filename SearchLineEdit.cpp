#include "SearchLineEdit.h"
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>

SearchLineEdit::SearchLineEdit(const QPixmap& pxLabel, const QPixmap& pxSearch,
                               QWidget* parent)
    : ButtonLineEdit(parent)
{
    QLabel* searchLabel = new QLabel(this);
    searchLabel->setPixmap(pxLabel);
    searchLabel->resize(16, 16);
    addLeftWidget(searchLabel, 0);

    ClearButton* btClear = new ClearButton(this);
    btClear->resize(16, 16);
    btClear->setToolTip(tr("Clear"));
    btClear->setShortcut(QKeySequence(Qt::Key_Escape));
    addRightWidget(btClear, 1);

    _btFullText = new PictureButton(this);
    _btFullText->setPixmap(pxSearch);
    _btFullText->setToolTip(tr("Full text search"));
    _btFullText->setShortcut(QKeySequence("Ctrl+Return"));
    _btFullText->resize(16, 16);
    addRightWidget(_btFullText, 0);

    // use a "invisible" button to grab shortcut for the lineedit
    QPushButton* btFocus = new QPushButton("Focus", this);
    btFocus->setShortcut(QKeySequence("Ctrl+F"));
    btFocus->setMaximumWidth(0);    // "hide" it
    addRightWidget(btFocus, 1);

    QLineEdit* lineEdit = getLineEdit();
    connect(btClear,  SIGNAL(clicked()),            lineEdit, SLOT(clear()));
    connect(lineEdit, SIGNAL(textChanged(QString)), btClear,  SLOT(onTextChanged(QString)));

    connect(lineEdit,    SIGNAL(textChanged(QString)), this, SIGNAL(filter(QString)));
    connect(_btFullText, SIGNAL(clicked()),            this, SLOT(onFullText()));
    connect(btFocus,     SIGNAL(clicked()),            this, SLOT(onFocus()));
}

void SearchLineEdit::setShowFullTextSearch(bool show) {
    _btFullText->setVisible(show);
}

void SearchLineEdit::onFullText() {
    emit fullTextSearch(getLineEdit()->text());
}

void SearchLineEdit::onFocus()
{
    if(getLineEdit()->hasFocus())
    {
        getLineEdit()->clear();
        clearFocus();
    }
    else
        setFocus();
}
