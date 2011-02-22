#include "ToolBar.h"
#include "OptionDlg.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

ToolBar::ToolBar(QWidget *parent)
	: QToolBar(parent)
{
	actionSmallIcon = new QAction(tr("Use small icons"), this);
	actionShowText  = new QAction(tr("Show text"), this);
	actionSmallIcon->setCheckable(true);
	actionShowText ->setCheckable(true);
	onSmallIcon(MySetting<UserSetting>::getInstance()->value("SmallIcon").toBool());
	onShowText (MySetting<UserSetting>::getInstance()->value("ShowText").toBool());
	connect(actionSmallIcon, SIGNAL(triggered(bool)), this, SLOT(onSmallIcon(bool)));
	connect(actionShowText,  SIGNAL(triggered(bool)), this, SLOT(onShowText(bool)));
}

void ToolBar::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);
	menu.addAction(actionSmallIcon);
	menu.addAction(actionShowText);
	menu.exec(event->globalPos());
}

void ToolBar::onSmallIcon(bool small)
{
	setIconSize(small ? QSize(24, 24) : QSize(48, 32));
	actionSmallIcon->setChecked(small);
	MySetting<UserSetting>::getInstance()->setValue("SmallIcon", small);
}

void ToolBar::onShowText(bool show)
{
	setToolButtonStyle(show ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
	actionShowText->setChecked(show);
	MySetting<UserSetting>::getInstance()->setValue("ShowText", show);
}

void ToolBar::initSearchBar()
{
    addSeparator();
    addWidget(new QLabel(tr(" Search ")));
    addWidget(leSearch = new QLineEdit(this));
	addWidget(new QLabel(" "));
	addWidget(cbFullText = new QCheckBox("Full text "));
	QPushButton* btClear = new QPushButton(QIcon(":/MainWindow/Images/Cancel.png"), QString(), this);
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

void ToolBar::onClear()
{
    leSearch->clearFocus();
    leSearch->clear();
    emit search(QString());
}

void ToolBar::onFocus()
{
    if(leSearch->hasFocus())
    {
        onClear();
        leSearch->clearFocus();
    }
    else
        leSearch->setFocus();
}
