#include "ToolBar.h"
#include "OptionDlg.h"
#include <QMenu>
#include <QContextMenuEvent>

MainToolBar::MainToolBar(QWidget *parent)
	: QToolBar(parent)
{
	// actions for the context menu
	actionSmallIcon = new QAction(tr("Use small icons"), this);
	actionShowText  = new QAction(tr("Show text"), this);
	actionSmallIcon->setCheckable(true);
	actionShowText ->setCheckable(true);
	connect(actionSmallIcon, SIGNAL(triggered(bool)), this, SLOT(onSmallIcon(bool)));
	connect(actionShowText,  SIGNAL(triggered(bool)), this, SLOT(onShowText (bool)));

    onSmallIcon(UserSetting::getInstance()->value("SmallIcon").toBool());
    onShowText (UserSetting::getInstance()->value("ShowText") .toBool());
}

void MainToolBar::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);
	menu.addAction(actionSmallIcon);
	menu.addAction(actionShowText);
	menu.exec(event->globalPos());
}

void MainToolBar::onSmallIcon(bool small)
{
	setIconSize(small ? QSize(24, 24) : QSize(48, 32));
	actionSmallIcon->setChecked(small);
    UserSetting::getInstance()->setValue("SmallIcon", small);
}

void MainToolBar::onShowText(bool show)
{
	setToolButtonStyle(show ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
	actionShowText->setChecked(show);
    UserSetting::getInstance()->setValue("ShowText", show);
}
