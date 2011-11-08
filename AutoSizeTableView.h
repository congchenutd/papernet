#ifndef AUTOSIZETABLEVIEW_H
#define AUTOSIZETABLEVIEW_H

#include "OptionDlg.h"
#include <QTableView>

// A TableView that saves the sizes of its columns
class AutoSizeTableView : public QTableView
{
	Q_OBJECT
	typedef QMap<int, double> Sizes;             // section, pertentage

public:
	AutoSizeTableView(QWidget *parent);
	void saveSectionSizes();                     // must be called before the dtr
	void init(const QString& parentObjectName);  // must be called right after the ctr

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void adjustColumns();

private:
	Sizes sectionSizes;
	UserSetting* setting;
	QString groupName;        // name of the parent table
	int adjustingCount;       // UGLY: how many resizeEvent, used to skip first 2 events
};

#endif // AUTOSIZETABLEVIEW_H
