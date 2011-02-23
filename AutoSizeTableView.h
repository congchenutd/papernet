#ifndef AUTOSIZETABLEVIEW_H
#define AUTOSIZETABLEVIEW_H

#include "OptionDlg.h"
#include <QTableView>

class AutoSizeTableView : public QTableView
{
	Q_OBJECT

public:
	AutoSizeTableView(QWidget *parent);

protected:
	virtual void resizeEvent(QResizeEvent*);
	virtual void closeEvent(QCloseEvent*);

private:
	QMap<int, double> sectionSizes;
	UserSetting* setting;
};

#endif // AUTOSIZETABLEVIEW_H
