#include "AutoSizeTableView.h"
#include <QHeaderView>
#include <QSettings>

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
    if(_adjustingCount++ >= 2)   // UGLY: skip the first 2 events during initialization
        adjustColumns();         // apply the sizes
	QTableView::resizeEvent(event);
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
	int lastNonZero = 0;
	int usedWidth   = 0;
    for(int i = 0; i < model()->columnCount(); ++i)
	{
        int colWidth = _sectionSizes.value(i) * width();
		if(colWidth > 0)
		{
			setColumnWidth(i, colWidth);
			lastNonZero = i;
			usedWidth += colWidth;
		}
	}

	// last non zero column takes unused width
	setColumnWidth(lastNonZero, width() - usedWidth);
}
