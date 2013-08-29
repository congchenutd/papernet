#include "ToolBar.h"
#include "OptionDlg.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QApplication>
#include <QPainter>
#include <QDebug>

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
    addWidget(_leSearch = new SearchLineEdit(this));

    _btFullText = new QPushButton(QIcon(":/Images/FullText.png"), QString());
    _btFullText->setToolTip(tr("Full text search"));
    addWidget(_btFullText);

	// use an invisible button to capture shortcut for the lineedit
	QPushButton* btFocus = new QPushButton("Focus", this);
	btFocus->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
	btFocus->setMaximumWidth(0);    // "hide" it
	addWidget(btFocus);

    _btClear = new ClearButton(this);
    _btClear->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(_btClear, SIGNAL(clicked()), this, SLOT(onClear()));
    connect(_leSearch, SIGNAL(textChanged(QString)),
            _btClear, SLOT(onTextChanged(QString)));

    connect(_leSearch,   SIGNAL(textEdited(QString)), this, SLOT(onSearch(QString)));
    connect(_btFullText, SIGNAL(clicked()),           this, SLOT(onFullTextSearch()));
    connect(btFocus,     SIGNAL(clicked()),           this, SLOT(onFocus()));
}

void SearchBar::resizeEvent(QResizeEvent* event)
{
    int margin = (_leSearch->height() - _btClear->height()) / 2;
    _btClear->move(_leSearch->x() + _leSearch->width() - _btClear->width() - margin,
                   _leSearch->y() + margin);
    QToolBar::resizeEvent(event);
}

void SearchBar::onClear()
{
    _leSearch->clearFocus();
    _leSearch->clear();
    _btClear->hide();
	emit clearSearch();
}

// Ctrl+F gets or releases focus
void SearchBar::onFocus()
{
    if(_leSearch->hasFocus())
		onClear();
	else
        _leSearch->setFocus();
}

void SearchBar::onFullTextSearch() {
    emit fullTextSearch(_leSearch->text());
}

void SearchBar::onSearch(const QString &target)
{
	if(target.isEmpty())
		onClear();
	else
        emit search(target);
}


//////////////////////////////////////////////////////////////////////////
SearchLineEdit::SearchLineEdit(QWidget* parent) 
    : QLineEdit(parent)
{
    _strHint = tr(" type to filter ...");
	clear();
}

void SearchLineEdit::clear()
{
	QPalette p = palette();                // show tip in gray font color
	p.setColor(QPalette::Text, Qt::gray);
	setPalette(p);
    setText(_strHint);
}

void SearchLineEdit::focusInEvent(QFocusEvent* event)
{
    if(text() == _strHint)            // empty
	{
		setPalette(qApp->palette());  // reset palette, use default color
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


//////////////////////////////////////////////////////////////////////////
ClearButton::ClearButton(QWidget* parent)
  : QAbstractButton(parent)
{
    setCursor(Qt::ArrowCursor);
    setToolTip(tr("Clear"));
    setFocusPolicy(Qt::NoFocus);
    hide();
}

void ClearButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(Qt::gray), 2));
    const int margin = 4;
    painter.drawLine(margin, margin, width() - margin, height() - margin);
    painter.drawLine(margin, height() - margin, width() - margin, margin);
}

void ClearButton::onTextChanged(const QString& text) {
    setVisible(!text.isEmpty());
}
