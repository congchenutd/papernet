#include "PageDictionary.h"
#include "Common.h"
#include "AddPhraseDlg.h"
#include "AddTagDlg.h"
#include <QMessageBox>
#include <QSqlQuery>

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

	ui.widgetWordCloud->setTableNames("DictionaryTags", "PhraseTag");
	ui.widgetWordCloud->updateSizes();   // init the size of the labels

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
	currentPhraseID = valid ? model.data(model.index(currentRow, TAG_ID)).toInt() : -1;
	emit tableValid(valid);
}

void PageDictionary::onAddTag()
{
	AddTagDlg dlg("DictionaryTags", this);
	if(dlg.exec() == QDialog::Accepted && !dlg.getText().isEmpty())
	{
		int tagID = getNextID("DictionaryTags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());
		ui.widgetWordCloud->addTagToPaper(tagID, currentPhraseID);
	}
}

void PageDictionary::onAddTagToPhrase()
{
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)
		ui.widgetWordCloud->addTagToPaper(getTagID("DictionaryTags", tag->text()),
										  currentPhraseID);
	highLightTags();
}

void PageDictionary::onDelTagFromPhrase()
{
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)
		ui.widgetWordCloud->removeTagFromPaper(getTagID("DictionaryTags", tag->text()), 
												currentPhraseID);
	highLightTags();
}

void PageDictionary::onFilterPhrases()
{
	QStringList tagClauses;
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)
		tagClauses << tr("Tag = %1").arg(getTagID("DictionaryTags", tag->text()));
	model.setFilter(tr("ID in (select Paper from PhraseTag where %1)")
									.arg(tagClauses.join(" OR ")));
}

void PageDictionary::highLightTags()
{
	ui.widgetWordCloud->unselectAll();
	ui.widgetWordCloud->highLight(getTagsOfPhrase(currentPhraseID));
}
