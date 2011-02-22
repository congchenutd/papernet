#include "ToolBar.h"
#include "OptionDlg.h"
#include <QMenu>
#include <QContextMenuEvent>

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