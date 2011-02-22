#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>

class QAction;
class QLineEdit;
class QPushButton;

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

public slots:
    void onClear();

private slots:
	void onSmallIcon(bool small);
	void onShowText(bool show);
    void onFocus();

private:
    QAction*   actionSmallIcon;
    QAction*   actionShowText;
    QLineEdit* leSearch;
};

#endif // TOOLBAR_H
