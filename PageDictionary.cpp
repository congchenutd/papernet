#include "PageDictionary.h"
#include "Common.h"

PageDictionary::PageDictionary(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	model.setTable("Dictionary");
	model.select();

	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(DICTIONARY_ID);
	ui.tableView->resizeColumnToContents(DICTIONARY_PHRASE);
}
