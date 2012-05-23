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
	int i = model()->columnCount() - 1;
	if(horizontalHeader()->stretchLastSection())
	{
		for(; i >=0; --i)
			if(sectionSizes.value(i) > 0)     // find and skip the last non-zero column
				break;
	}

	for(; i >= 0; --i)                        // apply the sizes
		setColumnWidth(i, width() * sectionSizes.value(i));
}
