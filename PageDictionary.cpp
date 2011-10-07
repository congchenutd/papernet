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

	onResetPhrases();   // init model

	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(DICTIONARY_ID);
	ui.tableView->hideColumn(DICTIONARY_PROXIMITY);
	ui.tableView->resizeColumnToContents(DICTIONARY_PHRASE);
	ui.tableView->sortByColumn(DICTIONARY_PHRASE, Qt::AscendingOrder);

	ui.widgetWordCloud->setTableNames("DictionaryTags", "PhraseTag", "Phrase");

	ui.splitter->restoreState(UserSetting::getInstance()->value("SplitterDictionary").toByteArray());

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
	connect(ui.tableView, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
	connect(ui.tableView, SIGNAL(showRelated()),              this, SLOT(onShowRelated()));
	connect(ui.widgetWordCloud, SIGNAL(filter()),    this, SLOT(onFilterPhrases()));
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),  this, SLOT(onResetPhrases()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),    this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),    this, SLOT(onAddTagToPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()), this, SLOT(onDelTagFromPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));
}

void PageDictionary::add()
{
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
		submit();
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
			delPhrase(getID(idx.row()));
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
		submit();
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
}

void PageDictionary::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	currentRow = valid ? idxList.front().row() : -1;
	currentPhraseID = valid ? model.data(model.index(currentRow, TAG_ID)).toInt() : -1;
	highLightTags();
	emit tableValid(valid);
}

// new tag
void PageDictionary::onAddTag()
{
	AddTagDlg dlg("DictionaryTags", this);
	if(dlg.exec() == QDialog::Accepted && !dlg.getText().isEmpty())
	{
		int tagID = getNextID("DictionaryTags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());
		ui.widgetWordCloud->addTagToItem(tagID, currentPhraseID);
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

void PageDictionary::onFilterPhrases()
{
	QStringList tagClauses;
	QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
	foreach(WordLabel* tag, tags)   // create filter from selected tags
		tagClauses << tr("Tag = %1").arg(getTagID("DictionaryTags", tag->text()));
	model.setFilter(tr("ID in (select Phrase from PhraseTag where %1)")
									.arg(tagClauses.join(" OR ")));
}

void PageDictionary::highLightTags()
{
	ui.widgetWordCloud->unselectAll();
	ui.widgetWordCloud->highLight(getTagsOfPhrase(currentPhraseID));
}

void PageDictionary::saveGeometry()
{
	UserSetting* setting = UserSetting::getInstance();
	setting->setValue("SplitterDictionary", ui.splitter->saveState());
}

void PageDictionary::onResetPhrases()
{
	model.setEditStrategy(QSqlTableModel::OnManualSubmit);
	model.setTable("Dictionary");
	model.select();
	while(model.canFetchMore())
		model.fetchMore();
}

// submit, while keep selecting current phrase
void PageDictionary::submit()
{
	int backupID = currentPhraseID;
	model.submitAll();
	jumpToID(backupID);
}

void PageDictionary::jumpToID(int id)
{
	int row = idToRow(&model, DICTIONARY_ID, id);
	if(row > -1)
	{
		currentRow = row;
		ui.tableView->selectRow(currentRow);  // will trigger onCurrentRowChanged()
		ui.tableView->setFocus();
	}
}

void PageDictionary::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		onResetPhrases();
	else
		onFilterPhrases();
}

void PageDictionary::search(const QString& target)
{
	if(target.isEmpty())
		onResetPhrases();
	else
		model.setFilter(tr("Phrase like \"%%1%\" or \
						    Explanation like \"%%1%\" ").arg(target));
}

void PageDictionary::onClicked(const QModelIndex& idx) {
	Navigator::getInstance()->addFootStep(this, getID(idx.row()));   // navigation
}

int PageDictionary::getID(int row) const {
	return model.data(model.index(row, DICTIONARY_ID)).toInt();
}

void PageDictionary::onShowRelated()
{
	QSqlDatabase::database().transaction();
	QSqlQuery query, subQuery;
	query.exec(tr("update Dictionary set Proximity = 0"));   // reset proximity

	// calculate proximity
	query.exec(tr("select Dictionary.ID, count(Phrase) Proximity from Dictionary, PhraseTag \
				  where Tag in (select Tag from PhraseTag where Phrase = %1) \
				  and Phrase != %1 and ID = Phrase \
				  group by Phrase").arg(currentPhraseID));

	// save proximity
	while(query.next()) {
		subQuery.exec(tr("update Dictionary set Proximity = %1 where ID = %2")
			.arg(query.value(1).toInt())
			.arg(query.value(0).toInt()));
	}

	// set itself the max proximity
	query.exec(tr("update Dictionary set Proximity = (select max(Proximity)+1 from Dictionary) \
				  where ID = %1").arg(currentPhraseID));
	QSqlDatabase::database().commit();

	ui.tableView->sortByColumn(DICTIONARY_PROXIMITY, Qt::DescendingOrder);  // sort
	jumpToID(currentPhraseID);                                              // keep highlighting
}