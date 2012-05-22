#ifndef Page_h__
#define Page_h__

#include <QWidget>

// An interface of pages
class Page : public QWidget
{
public:
	Page(QWidget* parent = 0) : QWidget(parent) {}
	virtual ~Page(void) {}

	virtual void add() = 0;
	virtual void del() = 0;
	virtual void search(const QString&) = 0;
	virtual void reset() = 0;              // reset the model
	virtual void jumpToID(int id) = 0;     // "click" on an row
    virtual void jumpToCurrent() = 0;
    virtual void enter() {}                // what to do when entered
};
#endif // Page_h__
