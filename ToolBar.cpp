#include "ToolBar.h"
#include "OptionDlg.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QApplication>

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

	onSmallIcon(MySetting<UserSetting>::getInstance()->value("SmallIcon").toBool());
	onShowText (MySetting<UserSetting>::getInstance()->value("ShowText").toBool());
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
	MySetting<UserSetting>::getInstance()->setValue("SmallIcon", small);
}

void MainToolBar::onShowText(bool show)
{
	setToolButtonStyle(show ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
	actionShowText->setChecked(show);
	MySetting<UserSetting>::getInstance()->setValue("ShowText", show);
}

//////////////////////////////////////////////////////////////////////////
SearchBar::SearchBar(QWidget* parent) : QToolBar(parent)
{
	addWidget(leSearch = new SearchLineEdit(this));
	addWidget(new QLabel(" "));

	btFullText = new QPushButton(QIcon(":/MainWindow/Images/FullText.png"), QString());
	btFullText->setToolTip(tr("Full text search"));
	addWidget(btFullText);

	QPushButton* btClear = new QPushButton(QIcon(":/MainWindow/Images/Cancel.png"), QString(), this);
	btClear->setShortcut(QKeySequence(Qt::Key_Escape));
	btClear->setToolTip(tr("Clear search result"));
	addWidget(btClear);

	// use an invisible button to capture shortcut for the lineedit
	QPushButton* btFocus = new QPushButton("Focus", this);
	btFocus->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
	btFocus->setMaximumWidth(0);    // "hide" it
	addWidget(btFocus);

	connect(leSearch,   SIGNAL(textEdited(QString)), this, SIGNAL(search(QString)));
	connect(btFullText, SIGNAL(clicked()),           this, SLOT(onFullTextSearch()));
	connect(btClear,    SIGNAL(clicked()),           this, SLOT(onClear()));
	connect(btFocus,    SIGNAL(clicked()),           this, SLOT(onFocus()));
}

void SearchBar::onClear()
{
	leSearch->clearFocus();
	leSearch->clear();
	emit clearSearch();
}

void SearchBar::onFocus()
{
	if(leSearch->hasFocus())
		onClear();
	else
		leSearch->setFocus();
}

void SearchBar::onFullTextSearch() {
	emit fullTextSearch(leSearch->text());
}

//////////////////////////////////////////////////////////////////////////
SearchLineEdit::SearchLineEdit(QWidget* parent) 
	: QLineEdit(parent)
{
	clear();
}

void SearchLineEdit::clear()
{
	QPalette p = palette();
	p.setColor(QPalette::Text, Qt::gray);
	setPalette(p);
	setText(tr(" type to search ..."));
}

void SearchLineEdit::focusInEvent(QFocusEvent* event)
{
	if(text() == tr(" type to search ..."))
	{
		setPalette(qApp->palette());  // reset palette
		QLineEdit::clear();
	}
	QLineEdit::focusInEvent(event);
}

void SearchLineEdit::focusOutEvent(QFocusEvent* event)
{
	if(text().isEmpty())
		clear();
	QLineEdit::focusOutEvent(event);
}
