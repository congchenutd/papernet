#include "Navigator.h"

Navigator::Navigator(QObject* parent) : QObject(parent) {}

void Navigator::addFootStep(Page* page, int id)
{
	// avoid duplication
	if(!history.isEmpty())
	{
		FootStep top = history.top();
		if(top.page == page && top.id == id)
        {
            emit historyValid(validateHistory());
			return;
        }
	}

	history.push(FootStep(page, id));
	emit historyValid(validateHistory());
}

FootStep Navigator::backward()
{
	if(!validateHistory())
		return FootStep();

	// move the top of history to future
	future.push(history.pop());

	emit historyValid(validateHistory());   // history may become valid
	emit futureValid (validateFuture());    // future  may become invalid

	// return the new top
	return history.top();
}

FootStep Navigator::forward()
{
	FootStep result;
	if(!validateFuture())    // no future
		return result;

	// move the top of future to history
	result = future.pop();
	history.push(result);

	emit historyValid(validateHistory());   // history may become valid
	emit futureValid (validateFuture());    // future  may become invalid

	// return the new top
	return result;
}

bool Navigator::validateHistory() {
    return history.size() > 1;   // at lease one footstep (current)
}

bool Navigator::validateFuture() {
	return !future.isEmpty();
}

Navigator* Navigator::getInstance()
{
	if(instance == 0)
		instance = new Navigator;
	return instance;
}

Navigator* Navigator::instance = 0;
