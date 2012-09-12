#include "AutoSizeTableView.h"

AutoSizeTableView::AutoSizeTableView(QWidget *parent)
	: QTableView(parent), setting(0), adjustingCount(0) {}

void AutoSizeTableView::init(const QString& parentObjectName)
{
	groupName = parentObjectName + "SectionSizes";
	setting = UserSetting::getInstance();
	setting->beginGroup(groupName);
	QStringList keys = setting->allKeys();
	foreach(QString key, keys)
		sectionSizes[key.toInt()] = setting->value(key).toFloat();
	setting->endGroup();
}

void AutoSizeTableView::resizeEvent(QResizeEvent* event)
{
	if(adjustingCount++ < 2)   // UGLY: skip the first 2 events during initialization
		adjustColumns();
	QTableView::resizeEvent(event);
}

void AutoSizeTableView::saveSectionSizes()
{
	setting->beginGroup(groupName);
	for(int i = 0; i < model()->columnCount(); ++i)
	{
		if(isColumnHidden(i))
			setting->setValue(QString::number(i), 0);
		else
		{
			double ratio = (double)(horizontalHeader()->sectionSize(i)) / width();
			setting->setValue(QString::number(i), qMax(0.01, qMin(0.9, ratio)));
		}
	}
	setting->endGroup();
}

void AutoSizeTableView::adjustColumns()
{
	int lastNonZero = 0;
	int usedWidth   = 0;
	const int columnCount = model()->columnCount();
	for(int i = 0; i < columnCount; ++i)
	{
		const int colWidth = sectionSizes.value(i) * width();
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
