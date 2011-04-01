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
	void saveSectionSizes();
	void init(const QString& parentObjectName);

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void resizeColumns();

private:
	Sizes sectionSizes;
	UserSetting* setting;
	QString groupName;
	int adjustTimes;
};

#endif // AUTOSIZETABLEVIEW_H
