#include "AutoSizeTableView.h"
#include <QHeaderView>
#include <QSettings>

AutoSizeTableView::AutoSizeTableView(QWidget* parent)
    : QTableView(parent), _setting(0), _loaded(false)
{}

void AutoSizeTableView::init(const QString& tableName, QSettings* setting)
{
    _setting = setting;
    _tableName = tableName;
    loadSectionSizes();
}

void AutoSizeTableView::showEvent(QShowEvent*) {
    if(!_loaded)
	{
        loadSectionSizes();
		_loaded = true;
	}
}

void AutoSizeTableView::loadSectionSizes()
{
    if(_setting != 0)
    {
        _setting->beginGroup("SectionSizes");
        horizontalHeader()->restoreState(_setting->value(_tableName).toByteArray());
        _setting->endGroup();
    }
}

void AutoSizeTableView::saveSectionSizes()
{
    if(_setting != 0)
    {
        _setting->beginGroup("SectionSizes");
        _setting->setValue(_tableName, horizontalHeader()->saveState());
        _setting->endGroup();
    }
}
