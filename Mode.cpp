#include "Mode.h"
#include "Common.h"
#include "AddPaperTagDlg.h"
#include "OptionDlg.h"
#include <QtGui>

Mode::Mode(PaperTagPage* p)
	: QWidget(p), page(p) {}

Mode::~Mode() {}

void Mode::enter()
{
	page->modelPapers.setTable("Papers");
	page->modelPapers.select();
	page->modelPapers.setEditStrategy(QSqlTableModel::OnFieldChange);
	page->modelTags.setTable("Tags");
	page->modelTags.select();
	page->resetViews();
}

PaperMode::PaperMode(PaperTagPage* page) : Mode(page)
{}

void PaperMode::enter()
{
	Mode::enter();

	page->ui.btImport->setEnabled(true);
	page->ui.btAddPaper->setEnabled(true);
	page->ui.btDelPaper->setEnabled(false);
}

void PaperMode::leave()
{
	page->ui.btImport->setEnabled(false);
	page->ui.btAddPaper->setEnabled(false);
	page->ui.btDelPaper->setEnabled(false);
	page->ui.btSave->setEnabled(false);
	page->ui.btCancel->setEnabled(false);
	page->ui.btSetPDF->setEnabled(false);
	page->ui.btReadPDF->setEnabled(false);
}

void PaperMode::addPaper()
{
	int lastRow = page->modelPapers.rowCount();
	page->modelPapers.insertRow(lastRow);
	page->modelPapers.setData(
		page->modelPapers.index(lastRow, PaperTagPage::PAPER_ID), getNextID("Papers", "ID"));
	page->ui.tableViewPapers->selectRow(lastRow);
	page->ui.leTitle->setFocus();
}

void PaperMode::delPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", QMessageBox::Yes | QMessageBox::No) 
		== QMessageBox::Yes)
	{
		bool keepPDF = MySetting<UserSetting>::getInstance()->getKeepPDF();
		QModelIndexList idxList = page->ui.tableViewPapers->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
		{
			int paperID = page->getPaperID(idx.row());
			if(!keepPDF)
				delPDF(paperID);
			::delPaper(paperID);
		}
		page->modelPapers.select();
	}
}

void PaperMode::addTag()
{
	AddPaperTagDlg dlg(this, "Tags");
	if(dlg.exec() == QDialog::Accepted)
	{
		QList<int> tags = dlg.getSelected();
		int paperID = page->getCurrentPaperID();
		foreach(int tagID, tags)
			addPaperTag(paperID, tagID);
		updateTags();
	}
}

void PaperMode::delTag()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", QMessageBox::Yes | QMessageBox::No) 
		== QMessageBox::Yes)
	{
		int id = page->getCurrentPaperID();
		QModelIndexList idxList = page->ui.listViewTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPaperTag(id, page->getTagID(idx.row()));
		updateTags();
	}
}

void PaperMode::updateTags()
{
	int id = page->getCurrentPaperID();
	page->modelTags.setFilter(tr("ID in (select Tag from PaperTag where Paper = %1) order by Name").arg(id));
	page->ui.listViewTags->setModelColumn(1);  // Name
}

//////////////////////////////////////////////////////////////////////////
// Tag
TagMode::TagMode(PaperTagPage* page) : Mode(page)
{}

void TagMode::enter()
{
	Mode::enter();

	page->ui.btAddTag->setEnabled(true);
}

void TagMode::leave()
{
	page->ui.btAddTag->setEnabled(false);
	page->ui.btEditTag->setEnabled(false);
	page->ui.btDelTag ->setEnabled(false);
}

void TagMode::addPaper()
{
	AddPaperTagDlg dlg(this, "Papers");
	if(dlg.exec() == QDialog::Accepted)
	{
		QList<int> papers = dlg.getSelected();
		int tag = page->getCurrentTagID();
		foreach(int paper, papers)
			addPaperTag(paper, tag);
		updatePapers();
	}
}

void TagMode::delPaper()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		int tag = page->getCurrentTagID();
		QModelIndexList idxList = page->ui.tableViewPapers->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			::delPaperTag(page->getPaperID(idx.row()), tag);
		updatePapers();
	}
}

void TagMode::updatePapers()
{
	QStringList tagClauses;
	QList<int> tags = page->getSelectedTagIDs();
	foreach(int tag, tags)
		tagClauses << tr("Tag = %1").arg(tag);
	QString temp = tr("ID in (select Paper from PaperTag where %1)").arg(tagClauses.join(" OR "));
	page->modelPapers.setFilter(tr("ID in (select Paper from PaperTag where %1)").arg(tagClauses.join(" OR ")));
}

void TagMode::addTag()
{
	bool ok;
	QString tag = QInputDialog::getText(this, "Add Tag", "Tag Name", 
		QLineEdit::Normal,	"New Tag", &ok);
	if(!ok || tag.isEmpty())
		return;
	int lastRow = page->modelTags.rowCount();
	page->modelTags.insertRow(lastRow);
	page->modelTags.setData(page->modelTags.index(lastRow, PaperTagPage::TAG_ID), getNextID("Tags", "ID"));
	page->modelTags.setData(page->modelTags.index(lastRow, PaperTagPage::TAG_NAME), tag);
	page->modelTags.submit();
}

void TagMode::editTag()
{
	QString tag = page->modelTags.data(
		page->modelTags.index(page->currentRowTag, PaperTagPage::TAG_NAME)).toString();
	bool ok;
	tag = QInputDialog::getText(this, "Edit Tag", "Tag Name", QLineEdit::Normal, tag, &ok);
	if(!ok || tag.isEmpty())
		return;
	page->modelTags.setData(page->modelTags.index(page->currentRowTag, PaperTagPage::TAG_NAME), tag);
	page->modelTags.submit();
}

void TagMode::delTag()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = page->ui.listViewTags->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			::delTag(page->getTagID(idx.row()));
		page->modelTags.select();
	}
}