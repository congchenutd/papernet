#include "AutoSizeTableView.h"

AutoSizeTableView::AutoSizeTableView(QWidget *parent)
	: QTableView(parent), setting(0) {}

void AutoSizeTableView::init(const QString& parentObjectName)
{
	this->groupName = parentObjectName + " Section Sizes";
	setting = MySetting<UserSetting>::getInstance();
	setting->beginGroup(groupName);
	QStringList keys = setting->allKeys();
	foreach(QString key, keys)
		sectionSizes[key.toInt()] = setting->value(key).toFloat();
	setting->endGroup();
}

void AutoSizeTableView::resizeEvent(QResizeEvent* event)
{
	int i = model()->columnCount() - 1;
	if(horizontalHeader()->stretchLastSection()) {
		while(i >= 0) {                       // find and skip last non-zero
			if(sectionSizes.value(i) > 0) {
				--i;
				break; 
			}
			--i;
		}
	}
	
	for(; i >= 0; --i)
		setColumnWidth(i, width() * sectionSizes.value(i));

	QTableView::resizeEvent(event);
}

void AutoSizeTableView::saveSectionSizes()
{
	setting->beginGroup(groupName);
	for(int i = 0; i < model()->columnCount(); ++i)
		setting->setValue(QString::number(i),
						  (double)(horizontalHeader()->sectionSize(i)) / width());
	setting->endGroup();
}