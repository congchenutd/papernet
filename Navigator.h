#ifndef Navigator_h__
#define Navigator_h__
#include <QStack>
#include <QObject>

class Page;

// navigation history record: page + record id
struct FootStep
{
	FootStep(Page* p = 0, int i = -1)
	{
		_page = p;
		_id   = i;
	}

	Page* _page;
	int   _id;      // record id on the page
};

// tracks navigation history
class Navigator : public QObject
{
	Q_OBJECT

public:
	Navigator(QObject* parent = 0);

	void addFootStep(Page* page, int row);
	FootStep backward();
	FootStep forward();

	static Navigator* getInstance();

signals:
	void historyValid(bool);
	void futureValid (bool);

private:
	bool validateHistory();
	bool validateFuture();

private:
	QStack<FootStep> _history;    // the top of history is the current position
	QStack<FootStep> _future;

	static Navigator* instance;
};
#endif // Navigator_h__
