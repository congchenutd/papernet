#include "SearchLineEdit.h"
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>

SearchLineEdit::SearchLineEdit(QWidget* parent)
    : ButtonLineEdit(parent),
      _label(0),
      _btClear(0),
      _btSearch(0)
{
    // use a "invisible" button to grab shortcut for the lineedit
    _btFocus = new QPushButton("Focus", this);
    _btFocus->setShortcut(QKeySequence("Ctrl+F"));
    _btFocus->setMaximumWidth(0);    // "hide" it
    addRightWidget(_btFocus);

    connect(getLineEdit(), SIGNAL(textChanged(QString)), this, SIGNAL(filter(QString)));
    connect(_btFocus,      SIGNAL(clicked()),            this, SLOT(onFocus()));
}

void SearchLineEdit::setLabel(QLabel* label)
{
    _label = label;
    _label->setParent(this);
    addLeftWidget(_label);
}

void SearchLineEdit::setClearButton(ClearButton* button)
{
    _btClear = button;
    _btClear->setParent(this);
    addRightWidget(_btClear);

    connect(_btClear, SIGNAL(clicked()), getLineEdit(),  SLOT(clear()));
    connect(getLineEdit(), SIGNAL(textChanged(QString)),
            _btClear, SLOT(onTextChanged(QString)));
}

void SearchLineEdit::setSearchButton(PictureButton* button)
{
    _btSearch = button;
    _btSearch->setParent(this);
    addRightWidget(_btSearch);
    connect(_btSearch, SIGNAL(clicked()), this, SLOT(onSearch()));
}

void SearchLineEdit::setShowSearchButton(bool show)
{
    if(_btSearch != 0)
        _btSearch->setVisible(show);
}

void SearchLineEdit::onSearch() {
    emit search(getLineEdit()->text());
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
