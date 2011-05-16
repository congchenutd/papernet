#ifndef PAPERTABLEVIEW_H
#define PAPERTABLEVIEW_H

#include <QTableView>
#include "AutoSizeTableView.h"

class PaperTableView : public AutoSizeTableView
{
	Q_OBJECT

public:
	PaperTableView(QWidget *parent);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
	void showRelated();
	void showCoauthored();
	void addQuote();
};

#endif // PAPERTABLEVIEW_H
