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

#endif // TOOLBAR_H
