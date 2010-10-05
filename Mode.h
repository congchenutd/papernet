#ifndef MODE_H
#define MODE_H

#include <QWidget>
#include "PaperTagPage.h"
class PaperTagPage;

class Mode : public QWidget
{
	Q_OBJECT

public:
	Mode(PaperTagPage* page);
	virtual ~Mode();

	virtual void enter();
	virtual void leave() = 0;
	virtual void addPaper() = 0;
	virtual void delPaper() = 0;
	virtual void addTag() = 0;
	virtual void editTag() {}
	virtual void delTag() = 0;
	virtual void updateTags() {}
	virtual void updatePapers() {}
	virtual QString getName() const = 0;

protected:
	PaperTagPage* page;
};

class PaperMode : public Mode
{
public:
	PaperMode(PaperTagPage* page);

	virtual void enter();
	virtual void leave();
	virtual void addPaper();
	virtual void delPaper();
	virtual void addTag();
	virtual void delTag();
	virtual void updateTags();
	virtual QString getName() const { return "PaperMode"; }
};

class TagMode : public Mode
{
public:
	TagMode(PaperTagPage* page);

	virtual void enter();
	virtual void leave();
	virtual void addPaper();
	virtual void delPaper();
	virtual void addTag();
	virtual void editTag();
	virtual void delTag();
	virtual void updatePapers();
	virtual QString getName() const { return "TagMode"; }
};

#endif // MODE_H
