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
    void initSearchBar();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
    void search(const QString& target);
	void fullTextSearch(const QString& target);

public slots:
    void onClear();
	void onFullTextSearch();

private slots:
	void onSmallIcon(bool small);
	void onShowText(bool show);
    void onFocus();

private:
    QAction*   actionSmallIcon;
    QAction*   actionShowText;
    QLineEdit* leSearch;
	QPushButton* btFullText;
};

#endif // TOOLBAR_H
