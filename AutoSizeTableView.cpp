#include "AutoSizeTableView.h"

AutoSizeTableView::AutoSizeTableView(QWidget *parent)
	: QTableView(parent)
{
	setting = MySetting<UserSetting>::getInstance();
	QStringList keys = setting->allKeys();
	foreach(QString key, keys)
		sectionSizes[key.toInt()] = setting->value(key).toFloat();
}

void AutoSizeTableView::resizeEvent(QResizeEvent* event)
{
	QTableView::resizeEvent(event);
	for(QMap<int, double>::iterator it = sectionSizes.begin(); it != sectionSizes.end(); ++it) 
		setColumnWidth(it.key(), width() * it.value());
}

void AutoSizeTableView::saveSectionSizes()
{
	for(int i = 0; i < model()->columnCount(); ++i)
		setting->setValue(QString::number(i), horizontalHeader()->sectionSize(i));
}
