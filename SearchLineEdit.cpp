#include "SearchLineEdit.h"
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>

SearchLineEdit::SearchLineEdit(QWidget* parent)
    : EmbeddableLineEdit(parent),
      _label(0),
      _btClear(0),
      _btSearch(0)
{
    connect(getLineEdit(), SIGNAL(textChanged(QString)), this, SIGNAL(filter(QString)));
}

void SearchLineEdit::setLabel(QLabel* label)
{
    if(label == 0)
        return;
    _label = label;
    _label->setParent(this);
    addLeftWidget(_label);
}

void SearchLineEdit::setClearButton(ClearButton* button)
{
    if(button == 0)
        return;
    _btClear = button;
    _btClear->setParent(this);
    addRightWidget(_btClear);

    connect(_btClear, SIGNAL(clicked()), this, SLOT(onClear()));
    connect(getLineEdit(), SIGNAL(textChanged(QString)),
            _btClear,      SLOT(onTextChanged(QString)));
}

void SearchLineEdit::setSearchButton(PictureButton* button)
{
    if(button == 0)
        return;
    _btSearch = button;
    _btSearch->setParent(this);
    addRightWidget(_btSearch);
    connect(_btSearch, SIGNAL(clicked()), this, SLOT(onSearch()));
}

void SearchLineEdit::setFocusShortcut(const QKeySequence& keySequence)
{
    QShortcut* shortcut = new QShortcut(keySequence, this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(onFocus()));
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

void SearchLineEdit::onClear()
{
    getLineEdit()->clear();
    emit(filter(QString()));
}
