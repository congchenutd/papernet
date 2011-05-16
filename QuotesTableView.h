#ifndef SNIPPETSTABLEVIEW_H
#define SNIPPETSTABLEVIEW_H

#include <QTableView>

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

#endif // SNIPPETSTABLEVIEW_H
