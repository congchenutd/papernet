#include "PageDictionary.h"
#include "Common.h"
#include "AddPhraseDlg.h"
#include "AddTagDlg.h"
#include <QMessageBox>

PageDictionary::PageDictionary(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	currentRow = -1;

	model.setEditStrategy(QSqlTableModel::OnManualSubmit);
	model.setTable("Dictionary");
	model.select();

	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(DICTIONARY_ID);
	ui.tableView->resizeColumnToContents(DICTIONARY_PHRASE);

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
	connect(ui.widgetWordCloud, SIGNAL(filter()),    this, SLOT(onFilterPhrases()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),    this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),    this, SLOT(onAddTagToPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()), this, SLOT(onDelTagFromPhrase()));
}

void PageDictionary::onAdd()
{
	AddPhraseDlg dlg(this);
	dlg.setWindowTitle(tr("Add Phrase"));
	if(dlg.exec() == QDialog::Accepted)
	{
		int lastRow = model.rowCount();
		model.insertRow(lastRow);
		model.setData(model.index(lastRow, DICTIONARY_ID),          getNextID("Dictionary", "ID"));
		model.setData(model.index(lastRow, DICTIONARY_PHRASE),      dlg.getPhrase());
		model.setData(model.index(lastRow, DICTIONARY_EXPLANATION), dlg.getExplanation());
		model.submitAll();
	}
}

void PageDictionary::onDel()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
			delPhrase(model.data(model.index(idx.row(), DICTIONARY_ID)).toInt());
		model.select();
	}
}

void PageDictionary::onEdit()
{
	AddPhraseDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Phrase"));
	dlg.setPhrase     (model.data(model.index(currentRow, DICTIONARY_PHRASE))     .toString());
	dlg.setExplanation(model.data(model.index(currentRow, DICTIONARY_EXPLANATION)).toString());
	if(dlg.exec() == QDialog::Accepted)
	{
		model.setData(model.index(currentRow, DICTIONARY_PHRASE),      dlg.getPhrase());
		model.setData(model.index(currentRow, DICTIONARY_EXPLANATION), dlg.getExplanation());
		model.submitAll();
	}
}

void PageDictionary::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRow = valid ? idxList.front().row() : -1;	
	emit tableValid(valid);
}

void PageDictionary::onAddTag()
{
	AddTagDlg dlg("DictionaryTags", this);
	if(dlg.exec() == QDialog::Accepted && !dlg.getText().isEmpty())
	{
		int tagID = getNextID("DictionaryTags", "ID");
		addTag("DictionaryTags", tagID, dlg.getText());
//		addPaperTag(currentPaperID, tagID);  // auto add this tag to current paper
//		addPhraseTag(cu)
		ui.widgetWordCloud->addWord(dlg.getText(), 20);
		ui.widgetWordCloud->updateSizes();
	}
}

void PageDictionary::onAddTagToPhrase()
{

}

void PageDictionary::onDelTagFromPhrase()
{

}

void PageDictionary::onFilterPhrases()
{

}