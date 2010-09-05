#include "TagList.h"
#include "Common.h"
#include "AddPaperTagDlg.h"
#include <QtGui>

TagList::TagList(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	modelTags.setTable("Tags");
	modelTags.select();

	ui.tableViewTags->setModel(&modelTags);
	ui.tableViewTags->hideColumn(TAG_ID);
	ui.tableViewTags->horizontalHeader()->setStretchLastSection(true);

	ui.tableViewPapers->setModel(&modelPapers);
	ui.tableViewRelatedTags->setModel(&modelRelatedTags);

	connect(ui.btAddTag,  SIGNAL(clicked()), this, SLOT(onAddTag()));
	connect(ui.btEditTag, SIGNAL(clicked()), this, SLOT(onEditTag()));
	connect(ui.btDelTag,  SIGNAL(clicked()), this, SLOT(onDelTag()));
	connect(ui.tableViewTags, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditTag()));
	connect(ui.tableViewTags->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onCurrentRowChanged(QModelIndex)));
	connect(ui.btAddPaper, SIGNAL(clicked()), this, SLOT(onAddPaper()));
	connect(ui.btDelPaper, SIGNAL(clicked()), this, SLOT(onDelPaper()));
	connect(ui.tableViewPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
			this, SLOT(onEnableDelPapers(QModelIndex)));
}

void TagList::onAddTag()
{
	bool ok;
	QString tag = QInputDialog::getText(this, "Add Tag", "Tag Name", 
		QLineEdit::Normal,	"New Tag", &ok);
	if(!ok || tag.isEmpty())
		return;
	int lastRow = modelTags.rowCount();
	modelTags.insertRow(lastRow);
	modelTags.setData(modelTags.index(lastRow, TAG_ID), getNextID("Tags", "ID"));
	modelTags.setData(modelTags.index(lastRow, TAG_NAME), tag);
	modelTags.submit();
}

void TagList::onEditTag()
{
	QString tag = modelTags.data(modelTags.index(currentRow, TAG_NAME)).toString();
	bool ok;
	tag = QInputDialog::getText(this, "Edit Tag", "Tag Name", QLineEdit::Normal, tag, &ok);
	if(!ok || tag.isEmpty())
		return;
	modelTags.setData(modelTags.index(currentRow, TAG_NAME), tag);
	modelTags.submit();
}

void TagList::onDelTag()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		delPaperTagByTag(getCurrentTagID());
		modelTags.removeRow(currentRow);
	}
}

void TagList::onCurrentRowChanged(const QModelIndex& idx)
{
	bool valid = idx.isValid();
	currentRow = valid ? idx.row() : -1;	
	ui.btEditTag->setEnabled(valid);
	ui.btDelTag ->setEnabled(valid);
	ui.btAddPaper->setEnabled(valid);
	ui.btDelPaper->setEnabled(false);
	
	if(valid)
	{
		updatePapers();
		updateRelatedTags();
	}
}

int TagList::getCurrentTagID() const {
	return modelTags.data(modelTags.index(currentRow, TAG_ID)).toInt();
}

void TagList::updateRelatedTags()
{
	int tagID = getCurrentTagID();
	modelRelatedTags.setQuery(tr("select Name, count(Tag) Proximity from Tags, PaperTag \
								   where Tag in (select Paper from PaperTag where Tag = %1) \
								   and Tag != %1 and ID = Tag \
								   group by Tag order by Proximity desc").arg(tagID));
	ui.tableViewRelatedTags->resizeColumnsToContents();
	ui.tableViewRelatedTags->horizontalHeader()->setStretchLastSection(true);
}

//////////////////////////////////////////////////////////////////////////
// Papers
void TagList::onAddPaper()
{
	AddPaperTagDlg dlg(this, "Papers");
	if(dlg.exec() == QDialog::Accepted)
	{
		QList<int> papers = dlg.getSelected();
		int tag = getCurrentTagID();
		foreach(int paper, papers)
			addPaperTag(paper, tag);
		updatePapers();
	}
}

void TagList::onDelPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		int tag = getCurrentTagID();
		QModelIndexList idxList = ui.tableViewPapers->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaperTag(modelPapers.data(modelPapers.index(idx.row(), 0)).toInt(), tag);

		updatePapers();
	}
}

void TagList::updatePapers()
{
	int tag = getCurrentTagID();
	modelPapers.setQuery(tr("select ID, Title, Authors, Journal from Papers \
			where ID in (select Paper from PaperTag where Tag = %1)").arg(tag));
	ui.tableViewPapers->hideColumn(0);
	ui.tableViewPapers->resizeColumnsToContents();
	ui.tableViewPapers->horizontalHeader()->setStretchLastSection(true);
}

void TagList::onEnableDelPapers(const QModelIndex& idx) {
	ui.btDelPaper->setEnabled(idx.isValid());
}
