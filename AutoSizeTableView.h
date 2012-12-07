#ifndef AUTOSIZETABLEVIEW_H
#define AUTOSIZETABLEVIEW_H

#include <QTableView>

class QSettings;

// A TableView that saves the sizes of its columns
class AutoSizeTableView : public QTableView
{
	Q_OBJECT

public:
    AutoSizeTableView(QWidget* parent);
    void init(const QString& tableName, QSettings* setting);  // call right after the ctr
    void saveSectionSizes();

protected:
    virtual void showEvent(QShowEvent *);

private:
    QSettings* _setting;
    QString    _tableName;       // name of the parent table
};

#endif // AUTOSIZETABLEVIEW_H
