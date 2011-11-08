#ifndef QUOTESTABLEVIEW_H
#define QUOTESTABLEVIEW_H

#include <QTableView>

// Support context menu
class QuotesTableView : public QTableView
{
	Q_OBJECT

public:
	QuotesTableView(QWidget *parent);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
	void addQuote();
	void delQuotes();
};

#endif // QUOTESTABLEVIEW_H
