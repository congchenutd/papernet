#include "PaperTagPage.h"

PaperTagPage::PaperTagPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	tableModelPaper.setTable("Papers");
	tableModelPaper.select();
	tableModelTag.setTable("Tags");
	tableModelTag.select();

	ui.listViewRelatedPapers->setModel(&modelRelatedPapers);
	ui.listViewRelatedTags  ->setModel(&modelRelatedTags);

	connect(ui.btByPaper, SIGNAL(clicked()), this, SLOT(onByPaper()));
	connect(ui.btByTag,   SIGNAL(clicked()), this, SLOT(onByTag()));
}

void PaperTagPage::onByPaper()
{
	ui.btByTag->setChecked(!ui.btByPaper->isChecked());
	mode = ui.btByPaper->isChecked() ? ByPaper : ByTag;

	ui.tableViewPapers->setModel(&tableModelPaper);
	ui.listViewTags->setModel(&queryModelTag);
	resetViews();
}

void PaperTagPage::onByTag()
{
	ui.btByPaper->setChecked(!ui.btByTag->isChecked());
	mode = ui.btByPaper->isChecked() ? ByPaper : ByTag;

	ui.listViewTags->setModel(&tableModelTag);
	ui.tableViewPapers->setModel(&queryModelPaper);
	resetViews();
}

void PaperTagPage::resetViews()
{
	ui.tableViewPapers->hideColumn(PAPER_ID);
	ui.tableViewPapers->hideColumn(PAPER_ABSTRACT);
	ui.tableViewPapers->hideColumn(PAPER_PDF);
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
	ui.listViewTags->setModelColumn(TAG_NAME);
}

