#include "AutoSizeTableView.h"
#include <QHeaderView>
#include <QSettings>


void AutoSizeLastColumnTableView::resizeEvent(QResizeEvent *)
{
    int totalWidth = width();
    for(int i = 0; i < model()->columnCount(); ++i)
        totalWidth -= columnWidth(i);

}


AutoSizeTableView::AutoSizeTableView(QWidget* parent)
    : QTableView(parent), _setting(0), _adjustingCount(0) {}

void AutoSizeTableView::init(const QString& parentObjectName, QSettings* setting)
{
    // load section sizes
    _setting = setting;
    _groupName = parentObjectName + "SectionSizes";
    _setting->beginGroup(_groupName);
    QStringList keys = _setting->allKeys();
	foreach(QString key, keys)
        _sectionSizes[key.toInt()] = _setting->value(key).toFloat();
    _setting->endGroup();
}

void AutoSizeTableView::resizeEvent(QResizeEvent* event)
{
    QTableView::resizeEvent(event);
    if(_adjustingCount++ >= 2)   // UGLY: skip the first 2 events during initialization
        adjustColumns();         // apply the sizes
}

void AutoSizeTableView::saveSectionSizes()
{
    _setting->beginGroup(_groupName);
	for(int i = 0; i < model()->columnCount(); ++i)
	{
		if(isColumnHidden(i))
            _setting->setValue(QString::number(i), 0);
		else
		{
			double ratio = (double)(horizontalHeader()->sectionSize(i)) / width();
            _setting->setValue(QString::number(i), qMax(0.01, qMin(0.9, ratio)));
		}
	}
    _setting->endGroup();
}

void AutoSizeTableView::adjustColumns()
{
    int lastNonZeroCol = model()->columnCount();
    for(int i = model()->columnCount(); i >= 0; --i)
        if(_sectionSizes.value(i) > 0)
        {
            lastNonZeroCol = i;
            break;
        }

    for(int i = 0; i < lastNonZeroCol; ++i)
        if(int colWidth = _sectionSizes.value(i) * width())
            setColumnWidth(i, colWidth);
}
