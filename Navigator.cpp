#include "Navigator.h"

Navigator::Navigator(QObject* parent) : QObject(parent) {}

void Navigator::addFootStep(Page* page, int id)
{
	// avoid duplication
	if(!_history.isEmpty())
	{
		FootStep top = _history.top();
		if(top._page == page && top._id == id)
			return;
	}

	_history.push(FootStep(page, id));
    validateHistory();
}

FootStep Navigator::backward()
{
	if(!validateHistory())
		return FootStep();

	// move the top of history to future
	_future.push(_history.pop());

    validateHistory();   // history may become invalid
    validateFuture();    // future  may become valid

	// return the new top
	return _history.top();
}

FootStep Navigator::forward()
{
	FootStep result;
	if(!validateFuture())    // no future
		return result;

	// move the top of future to history
	result = _future.pop();
	_history.push(result);

    validateHistory();   // history may become valid
    validateFuture();    // future  may become invalid

	// return the new top
	return result;
}

bool Navigator::validateHistory()
{
    bool valid = _history.size() > 1;  // at least one footstep besides the current one
    emit historyValid(valid);
    return valid;
}

bool Navigator::validateFuture()
{
    bool valid = !_future.isEmpty();
    emit futureValid(valid);
    return valid;
}

Navigator* Navigator::getInstance()
{
	if(instance == 0)
		instance = new Navigator;
	return instance;
}

Navigator* Navigator::instance = 0;
