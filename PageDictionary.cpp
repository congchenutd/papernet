#include "PageDictionary.h"
#include "Common.h"
#include "AddPhraseDlg.h"
#include "AddTagDlg.h"
#include "OptionDlg.h"
#include "Navigator.h"
#include <QMessageBox>
#include <QSqlQuery>

PageDictionary::PageDictionary(QWidget *parent)
	: Page(parent)
{
	ui.setupUi(this);
	currentRow = -1;
	currentPhraseID = -1;

	onResetPhrases();   // init model
	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(DICTIONARY_ID);
	ui.tableView->hideColumn(DICTIONARY_PROXIMITY);
	ui.tableView->resizeColumnToContents(DICTIONARY_PHRASE);
    ui.tableView->sortByColumn(DICTIONARY_PHRASE, Qt::AscendingOrder);

	ui.widgetWordCloud->setTableNames("DictionaryTags", "PhraseTag", "Phrase");
	loadGeometry();

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
	connect(ui.tableView, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
    connect(ui.widgetWordCloud, SIGNAL(filter(bool)), this, SLOT(onFilterByTags(bool)));
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),   this, SLOT(onResetPhrases()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagToPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));
    connect(ui.widgetRelated,   SIGNAL(doubleClicked(int)),     this, SLOT(onRelatedDoubleClicked(int)));
}

void PageDictionary::add()
{
	reset();
	AddPhraseDlg dlg(this);
	dlg.setWindowTitle(tr("Add Phrase"));
	if(dlg.exec() == QDialog::Accepted)
	{
		int lastRow = model.rowCount();
		currentPhraseID = getNextID("Dictionary", "ID");
		model.insertRow(lastRow);
		model.setData(model.index(lastRow, DICTIONARY_ID),          currentPhraseID);
		model.setData(model.index(lastRow, DICTIONARY_PHRASE),      dlg.getPhrase());
		model.setData(model.index(lastRow, DICTIONARY_EXPLANATION), dlg.getExplanation());
        model.submitAll();
		updateTags(dlg.getTags());
	}
}

void PageDictionary::del()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
            delPhrase(rowToID(idx.row()));
		model.select();
	}
}

void PageDictionary::onEdit()
{
	AddPhraseDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Phrase"));
	dlg.setPhrase     (model.data(model.index(currentRow, DICTIONARY_PHRASE))     .toString());
	dlg.setExplanation(model.data(model.index(currentRow, DICTIONARY_EXPLANATION)).toString());
	dlg.setTags(getTagsOfPhrase(currentPhraseID));
	if(dlg.exec() == QDialog::Accepted)
	{
		model.setData(model.index(currentRow, DICTIONARY_PHRASE),      dlg.getPhrase());
		model.setData(model.index(currentRow, DICTIONARY_EXPLANATION), dlg.getExplanation());
        model.submitAll();
		updateTags(dlg.getTags());
	}
}

// recreate the associations to tags
void PageDictionary::updateTags(const QStringList& tags)
{
	QSqlQuery query;
	query.exec(tr("delete from PhraseTag where Phrase = %1").arg(currentPhraseID));
	foreach(QString tag, tags)
	{
		if(tag.isEmpty())
			continue;
		int tagID = getTagID("DictionaryTags", tag);
		if(tagID < 0)
		{
			tagID = getNextID("DictionaryTags", "ID");
			ui.widgetWordCloud->addTag(tagID, tag);
		}
		ui.widgetWordCloud->addTagToItem(tagID, currentPhraseID);
	}
	highLightTags();
}

void PageDictionary::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRow = valid ? idxList.front().row() : -1;
	if(valid)
	{
		currentPhraseID = model.data(model.index(currentRow, TAG_ID)).toInt();
		highLightTags();
        ui.widgetRelated->setCentralPhraseID(currentPhraseID);
	}
	emit tableValid(valid);
}

void PageDictionary::onClicked(const QModelIndex& idx) {
    Navigator::getInstance()->addFootStep(this, rowToID(idx.row()));   // navigation
}

// new tag
void PageDictionary::onAddTag()
{
	AddTagDlg dlg("DictionaryTags", this);
	if(dlg.exec() == QDialog::Accepted)
	{
		int tagID = getNextID("DictionaryTags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());
		ui.widgetWordCloud->addTagToItem(tagID, currentPhraseID);
		highLightTags();
	}
}

// add selected tags to selected phrases
void PageDictionary::onAddTagToPhrase()
{
	QModelIndexList rows = ui.tableView->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
		int phraseID = model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->addTagToItem(getTagID("DictionaryTags", tag->text()),
											 phraseID);
	}
	highLightTags();
}

// del selected tags from selected phrases
void PageDictionary::onDelTagFromPhrase()
{
	QModelIndexList rows = ui.tableView->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
		int phraseID = model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->removeTagFromItem(getTagID("DictionaryTags", tag->text()),
												  phraseID);
	}
	highLightTags();
}

void PageDictionary::onFilterByTags(bool AND)
{
	// get selected tags
	QStringList tagIDs;
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)
		tagIDs << tr("%1").arg(getTagID("DictionaryTags", tag->text()));

	if(!AND)
		model.setFilter(
			tr("ID in (select Phrase from PhraseTag where Tag in (%1))").arg(tagIDs.join(",")));
	else
	{
		dropTempView();
		QSqlQuery query;    // create a temp table for selected tags id
		query.exec(tr("create view SelectedTags as select * from DictionaryTags\
					  where ID in (%1)").arg(tagIDs.join(",")));

		// select phrases that contain all the selected tags
		model.setFilter("not exists \
						 (select * from SelectedTags where \
						 not exists \
							 (select * from PhraseTag where \
							 phrase=Dictionary.ID and Tag=SelectedTags.ID))");
	}
}

void PageDictionary::highLightTags() {
	ui.widgetWordCloud->highLight(getTagsOfPhrase(currentPhraseID));
}

void PageDictionary::loadGeometry()
{
    UserSetting* setting = UserSetting::getInstance();
    ui.splitterVertical  ->restoreState(setting->getSplitterSizes("DictionaryVertical"));
    ui.splitterHorizontal->restoreState(setting->getSplitterSizes("DictionaryHorizontal"));
}

void PageDictionary::saveGeometry()
{
	UserSetting* setting = UserSetting::getInstance();
    setting->setSplitterSizes("DictionaryVertical",   ui.splitterVertical  ->saveState());
    setting->setSplitterSizes("DictionaryHorizontal", ui.splitterHorizontal->saveState());
}

void PageDictionary::onResetPhrases()
{
	model.setEditStrategy(QSqlTableModel::OnManualSubmit);
	model.setTable("Dictionary");
	model.select();
//	while(model.canFetchMore())
//		model.fetchMore();
	jumpToID(currentPhraseID);
}

void PageDictionary::jumpToID(int id)
{
	int row = idToRow(&model, DICTIONARY_ID, id);
	if(row > -1)
	{
		currentRow = row;
		ui.tableView->selectRow(currentRow);  // will trigger onCurrentRowChanged()
		ui.tableView->setFocus();
		ui.tableView->scrollTo(model.index(row, DICTIONARY_PHRASE));
	}
}

void PageDictionary::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		onResetPhrases();
	else
        onFilterByTags();
}

void PageDictionary::onRelatedDoubleClicked(int phraseID)
{
    jumpToID(phraseID);
    Navigator::getInstance()->addFootStep(this, phraseID);
}

void PageDictionary::search(const QString& target)
{
	model.setFilter(tr("Phrase like \"%%1%\" or \
						Explanation like \"%%1%\" ").arg(target));

	// highlight tags
	ui.widgetWordCloud->search(target);
}

int PageDictionary::rowToID(int row) const {
	return model.data(model.index(row, DICTIONARY_ID)).toInt();
}
