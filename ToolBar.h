#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QLineEdit>
#include <QAbstractButton>

class QAction;
class QLineEdit;
class QPushButton;
class QCheckBox;

// A tool bar with a context menu (show small icon, show text)
class MainToolBar : public QToolBar
{
	Q_OBJECT

public:
	MainToolBar(QWidget* parent = 0);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void onSmallIcon(bool small);
	void onShowText (bool show);

private:
    QAction* actionSmallIcon;
    QAction* actionShowText;
};

// A line edit that shows "type to filter" when it loses focus
class SearchLineEdit : public QLineEdit
{
public:
	SearchLineEdit(QWidget* parent = 0);
	void clear();

protected:
	virtual void focusInEvent (QFocusEvent*);
	virtual void focusOutEvent(QFocusEvent*);

private:
    QString _strHint;
};

// A search tool bar
class ClearButton;
class SearchBar : public QToolBar
{
	Q_OBJECT

public:
	SearchBar(QWidget* parent = 0);

public slots:
	void onClear();

private slots:
	void onFullTextSearch();
	void onFocus();
	void onSearch(const QString& target);

signals:
	void search(const QString& target);
	void clearSearch();
	void fullTextSearch(const QString& target);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    SearchLineEdit* _leSearch;
    ClearButton*    _btClear;
    QPushButton*    _btFullText;
};

///////////////////////////////////////////////////////////////////
/// a small X button embeded in a lineedit
///
class ClearButton : public QAbstractButton
{
    Q_OBJECT

public:
    ClearButton(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);

    int width()  const { return 16; }   // URGLY: hard coded
    int height() const { return 16; }

public slots:
    void onTextChanged(const QString& text);
};


#endif // TOOLBAR_H
