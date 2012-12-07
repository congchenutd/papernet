#ifndef AUTOSIZETABLEVIEW_H
#define AUTOSIZETABLEVIEW_H

#include <QTableView>

class QSettings;

class AutoSizeLastColumnTableView : public QTableView
{
protected:
    virtual void resizeEvent(QResizeEvent*);
};

// A TableView that saves the sizes of its columns
class AutoSizeTableView : public QTableView
{
	Q_OBJECT
	typedef QMap<int, double> Sizes;             // section, pertentage

public:
    AutoSizeTableView(QWidget* parent);
    void saveSectionSizes();                                         // call before the dtr
    void init(const QString& parentObjectName, QSettings* setting);  // call right after the ctr

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
    void adjustColumns();

private:
    Sizes      _sectionSizes;    // sizes (in percentage) of the sections
    QSettings* _setting;
    QString    _groupName;       // name of the parent table
    int        _adjustingCount;  // UGLY: how many resizeEvent, used to skip first 2 events
};

#endif // AUTOSIZETABLEVIEW_H
