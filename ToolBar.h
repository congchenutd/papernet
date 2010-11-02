#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>

class ToolBar : public QToolBar
{
	Q_OBJECT

public:
	ToolBar(QWidget* parent = 0);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void onSmallIcon(bool small);
	void onShowText(bool show);

private:
	QAction* actionSmallIcon;
	QAction* actionShowText;
};

#endif // TOOLBAR_H
