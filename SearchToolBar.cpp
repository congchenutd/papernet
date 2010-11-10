#include "SearchToolBar.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

SearchToolBar::SearchToolBar(QWidget *parent)
	: QToolBar(parent)
{
    addWidget(new QLabel(tr(" Search ")));
    addWidget(leSearch = new QLineEdit(this));
	btClear = new QPushButton(QIcon(":/MainWindow/Images/Cancel.png"), QString(), this);
	btClear->setShortcut(QKeySequence(Qt::Key_Escape));
	addWidget(btClear);
    QPushButton* btFocus = new QPushButton("Focus", this);
    btFocus->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    btFocus->setMaximumWidth(0);    // "hide" it
    addWidget(btFocus);

	connect(leSearch, SIGNAL(textEdited(QString)), this, SIGNAL(search(QString)));
	connect(btClear,  SIGNAL(clicked()),           this, SLOT(onClear()));
    connect(btFocus,  SIGNAL(clicked()),           this, SLOT(onFocus()));
}

void SearchToolBar::onClear()
{
    leSearch->clearFocus();
	leSearch->clear();
	emit search(QString());
}

void SearchToolBar::onFocus()
{
    if(leSearch->hasFocus())
    {
        onClear();
        leSearch->clearFocus();
    }
    else
        leSearch->setFocus();
}
