#ifndef PAPERTABLEVIEW_H
#define PAPERTABLEVIEW_H

#include <QTableView>

class PaperTableView : public QTableView
{
	Q_OBJECT

public:
	PaperTableView(QWidget *parent);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
	void showRelated();
	void showCoauthored();
	void addSnippet();
};

#endif // PAPERTABLEVIEW_H
