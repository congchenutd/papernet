#ifndef Page_h__
#define Page_h__

#include <QWidget>

// An interface of pages, for MainWindow's menu actions
class Page : public QWidget
{
public:
	Page(QWidget* parent = 0) : QWidget(parent) {}
	virtual ~Page(void) {}

	virtual void add() = 0;
	virtual void del() = 0;
	virtual void search(const QString&) {}
	virtual void jumpToID(int id) {}   // "click" an ID
};
#endif // Page_h__
