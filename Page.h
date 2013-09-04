#ifndef Page_h__
#define Page_h__

#include <QWidget>

// An interface of pages
class Page : public QWidget
{
public:
	Page(QWidget* parent = 0) : QWidget(parent) {}
	virtual ~Page(void) {}

    virtual void addRecord()  = 0;
    virtual void delRecord()  = 0;
    virtual void editRecord() = 0;
	virtual void search(const QString&) = 0;
    virtual void reset() = 0;           // reset the model
    virtual void jumpToID(int id) = 0;  // pages are responsible for visibility
    virtual void jumpToCurrent()  = 0;  // pages are responsible for visibility
};

#endif // Page_h__
