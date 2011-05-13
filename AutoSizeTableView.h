#ifndef AUTOSIZETABLEVIEW_H
#define AUTOSIZETABLEVIEW_H

#include "OptionDlg.h"
#include <QTableView>

class AutoSizeTableView : public QTableView
{
	Q_OBJECT
	typedef QMap<int, double> Sizes;

public:
	AutoSizeTableView(QWidget *parent);
	void saveSectionSizes();                     // must be called before the dtr
	void init(const QString& parentObjectName);  // FIXME: how to call after the ctr?

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void adjustColumns();

private:
	Sizes sectionSizes;
	UserSetting* setting;
	QString groupName;
	int adjustingCount;
};

#endif // AUTOSIZETABLEVIEW_H
