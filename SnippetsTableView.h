#ifndef SNIPPETSTABLEVIEW_H
#define SNIPPETSTABLEVIEW_H

#include <QTableView>

class SnippetsTableView : public QTableView
{
	Q_OBJECT

public:
	SnippetsTableView(QWidget *parent);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
	void addSnippet();
	void delSnippets();
};

#endif // SNIPPETSTABLEVIEW_H
