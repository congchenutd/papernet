#ifndef Navigator_h__
#define Navigator_h__
#include <QStack>
#include <QObject>

// navigation history record
class Page;
struct FootStep
{
	FootStep()
	{
		page = 0;
		id = -1;
	}

	FootStep(Page* p, int i)
	{
		page = p;
		id   = i;
	}

	Page* page;
	int   id;
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
	QStack<FootStep> history;    // the top of history is the current position
	QStack<FootStep> future;

	static Navigator* instance;
};
#endif // Navigator_h__
