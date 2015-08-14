#ifndef PAPERTABLEVIEW_H
#define PAPERTABLEVIEW_H

#include "AutoSizeTableView.h"

// Support context menu
class PaperTableView : public AutoSizeTableView
{
	Q_OBJECT

public:
	PaperTableView(QWidget *parent);
    void sortByColumn(int column, Qt::SortOrder order);
    void reSort();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

signals:
	void showRelated();
	void showCoauthored();
	void addQuote();
	void printMe(bool);
	void bookmark(bool);
	void addPDF();
	void readPDF();

private slots:
    void onSectionClicked(int logicalIndex, Qt::SortOrder order);

private:
	QAction* actionAddQuote;
	QAction* actionPrintMe;
	QAction* actionBookmark;
	QAction* actionAddPDF;
	QAction* actionReadPDF;

    int _sortedColumn;
    Qt::SortOrder _sortedOrder;
};

#endif // PAPERTABLEVIEW_H
