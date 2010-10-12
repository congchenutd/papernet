#include "PagePapers.h"
#include "Common.h"
#include "OptionDlg.h"
#include <QDataWidgetMapper>
#include <QMessageBox>

PagePapers::PagePapers(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.splitterHorizontal->setSizes(QList<int>() << width()  * 0.8 << width()  * 0.2);
	ui.splitterPapers    ->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);
	ui.splitterTags      ->setSizes(QList<int>() << height() * 0.5 << height() * 0.5);

	modelPapers.setTable("Papers");
	modelPapers.setEditStrategy(QSqlTableModel::OnFieldChange);
	modelPapers.select();
	modelAllTags.setTable("Tags");
	modelAllTags.setFilter("Name != \'\' order by Name");
	modelAllTags.setEditStrategy(QSqlTableModel::OnManualSubmit);
	modelAllTags.select();

	mapper = new QDataWidgetMapper(this);
	mapper->setModel(&modelPapers);
	mapper->addMapping(ui.leTitle,    PAPER_TITLE);
	mapper->addMapping(ui.leAuthors,  PAPER_AUTHORS);
	mapper->addMapping(ui.leJournal,  PAPER_JOURNAL);
	mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);

	ui.tableViewPapers->setModel(&modelPapers);
//	ui.tableViewPapers->hideColumn(PAPER_ID);
	ui.tableViewPapers->hideColumn(PAPER_ABSTRACT);
	ui.tableViewPapers->hideColumn(PAPER_PDF);
	ui.tableViewPapers->resizeColumnsToContents();
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
	ui.tableViewPapers->sortByColumn(PAPER_TITLE, Qt::AscendingOrder);

	ui.listViewAllTags->setModel(&modelAllTags);
	ui.listViewAllTags->setModelColumn(TAG_NAME);

	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowPaperChanged(QModelIndex)));
	connect(ui.tableViewPapers, SIGNAL(clicked(QModelIndex)),
			this, SLOT(onClick()));
	//connect(ui.listViewAllTags->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
	//		this, SLOT(onCurrentRowTagChanged()));

	connect(ui.btAddPaper, SIGNAL(clicked()), this, SLOT(onAddPaper()));
	connect(ui.btDelPaper, SIGNAL(clicked()), this, SLOT(onDelPaper()));
}

void PagePapers::onCurrentRowPaperChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	currentRowPapers = valid ? idx.row() : -1;
	ui.btDelPaper->setEnabled(valid);
	ui.btAddTag->setEnabled(valid);
	ui.btDelTag->setEnabled(false);
	ui.btSetPDF->setEnabled(valid);
}

void PagePapers::onAddPaper()
{
	int lastRow = modelPapers.rowCount();
	modelPapers.insertRow(lastRow);
	int nextID = getNextID("Papers", "ID");
	modelPapers.setData(modelPapers.index(lastRow, PAPER_ID), nextID);
	modelPapers.submit();
	selectID(nextID);
	ui.leTitle->setFocus();
}

void PagePapers::onDelPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		bool keepPDF = MySetting<UserSetting>::getInstance()->getKeepPDF();
		int paperID = getPaperID(currentRowPapers);
		if(!keepPDF)
			delPDF(paperID);
		::delPaper(paperID);
		modelPapers.select();
	}
}

int PagePapers::getPaperID(int row) const {
	return row > -1 ? modelPapers.data(modelPapers.index(row, PAPER_ID)).toInt() : -1;
}

void PagePapers::selectID(int id)
{
	QModelIndexList indexes = modelPapers.match(
		modelPapers.index(0, PAPER_ID), Qt::DisplayRole, id, 1, Qt::MatchExactly | Qt::MatchWrap);
	if(!indexes.isEmpty())
	{
		currentRowPapers = indexes.at(0).row();
		ui.tableViewPapers->selectRow(currentRowPapers);
	}
}