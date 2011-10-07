#ifndef DictionaryTableView_h__
#define DictionaryTableView_h__

#include "AutoSizeTableView.h"

class DictionaryTableView : public AutoSizeTableView
{
	Q_OBJECT

public:
	DictionaryTableView(QWidget* parent);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
	void showRelated();
};

#endif // DictionaryTableView_h__
