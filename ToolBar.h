#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>

class QAction;
class QLineEdit;
class QPushButton;
class QCheckBox;

class ToolBar : public QToolBar
{
	Q_OBJECT

public:
	ToolBar(QWidget* parent = 0);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void onSmallIcon(bool small);
	void onShowText (bool show);

private:
    QAction* actionSmallIcon;
    QAction* actionShowText;
};

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

signals:
	void search(const QString& target);
	void fullTextSearch(const QString& target);

private:
	QLineEdit*   leSearch;
	QPushButton* btFullText;
};

#endif // TOOLBAR_H
