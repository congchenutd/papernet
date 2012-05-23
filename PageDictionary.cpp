#include "PageDictionary.h"
#include "Common.h"
#include "AddPhraseDlg.h"
#include "AddTagDlg.h"
#include "OptionDlg.h"
#include "Navigator.h"
#include "Thesaurus.h"
#include <QMessageBox>
#include <QSqlQuery>

PageDictionary::PageDictionary(QWidget *parent)
	: Page(parent)
{
	ui.setupUi(this);
	currentRow = -1;
	currentPhraseID = -1;

	onResetPhrases();   // init model
	phraseThesaurus = new BigHugeThesaurus(this);
	tagThesaurus    = new BigHugeThesaurus(this);
	connect(phraseThesaurus, SIGNAL(response(QStringList)), this, SLOT(onPhraseThesaurus(QStringList)));
	connect(tagThesaurus,    SIGNAL(response(QStringList)), this, SLOT(onTagThesaurus   (QStringList)));

	ui.tableView->setModel(&model);
	ui.tableView->hideColumn(DICTIONARY_ID);
	ui.tableView->hideColumn(DICTIONARY_PROXIMITY);
	ui.tableView->resizeColumnToContents(DICTIONARY_PHRASE);
	sortByPhrase();

	ui.widgetWordCloud->setTableNames("DictionaryTags", "PhraseTag", "Phrase");
	loadGeometry();

	connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
	connect(ui.tableView, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked(QModelIndex)));
	connect(ui.tableView, SIGNAL(showRelated()),              this, SLOT(onShowRelated()));
	connect(ui.widgetWordCloud, SIGNAL(filter(bool)), this, SLOT(onFilterPhrasesByTags(bool)));
	connect(ui.widgetWordCloud, SIGNAL(unfilter()),   this, SLOT(onResetPhrases()));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onAddTag()));
	connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagToPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPhrase()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));
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
	}
	emit tableValid(valid);
}

void PageDictionary::onClicked(const QModelIndex& idx) {
	Navigator::getInstance()->addFootStep(this, getID(idx.row()));   // navigation
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

void PageDictionary::onFilterPhrasesByTags(bool AND)
{
	hideRelated();

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

void PageDictionary::loadGeometry() {
	ui.splitter->restoreState(UserSetting::getInstance()->getSplitterSizes("Dictionary"));
}

void PageDictionary::saveGeometry()
{
	UserSetting* setting = UserSetting::getInstance();
	setting->setSplitterSizes("Dictionary", ui.splitter->saveState());
}

void PageDictionary::onResetPhrases()
{
	hideRelated();   // reset coloring
	model.setEditStrategy(QSqlTableModel::OnManualSubmit);
	model.setTable("Dictionary");
	model.select();
	while(model.canFetchMore())
		model.fetchMore();
	sortByPhrase();
	jumpToID(currentPhraseID);
}

// submit, while keep selecting current phrase
void PageDictionary::submit()
{
	hideRelated();
	int backupID = currentPhraseID;
	model.submitAll();
	sortByPhrase();
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
		ui.tableView->scrollTo(model.index(row, DICTIONARY_PHRASE));
	}
}

void PageDictionary::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		onResetPhrases();
	else
		onFilterPhrasesByTags();
}

void PageDictionary::search(const QString& target)
{
	model.setFilter(tr("Phrase like \"%%1%\" or \
						Explanation like \"%%1%\" ").arg(target));

	// highlight tags
	ui.widgetWordCloud->search(target);
}

int PageDictionary::getID(int row) const {
	return model.data(model.index(row, DICTIONARY_ID)).toInt();
}

void PageDictionary::onShowRelated()
{
	onResetPhrases();

	// ------------ calculate proximity by tags -------------
	QSqlDatabase::database().transaction();

	// gather related tags: tags this phrase has (direct), and their proximate tags (from tagThesaurus)
	QSqlQuery query;    // query direct tags' names
	query.exec(tr("select Name from DictionaryTags, PhraseTag \
				   where Phrase = %1 and ID = Tag").arg(currentPhraseID));
	while(query.next())
		tagThesaurus->request(query.value(0).toString());   // query proximate tags with this direct tag

	// calculate proximity by direct tags:
	// 1. find in PhraseTag all direct tags
	// 2. count the # of all other phrases that have these tags
	// 3. associate the # with the phrases
	query.exec(tr("select Dictionary.ID, count(PhraseTag.Phrase) Proximity \
				  from Dictionary, PhraseTag \
				  where Tag in (select Tag from PhraseTag where Phrase = %1) \
						and ID != %1 and ID = PhraseTag.Phrase \
				  group by PhraseTag.Phrase").arg(currentPhraseID));

	// save proximity
	QSqlQuery subQuery;
	while(query.next()) {
		subQuery.exec(tr("update Dictionary set Proximity = %1 where ID = %2")
			.arg(query.value(1).toInt())
			.arg(query.value(0).toInt()));
	}

	// give itself the max proximity
	query.exec(tr("update Dictionary set Proximity = \
					  (select max(Proximity)+1 from Dictionary where ID <> %1) \
				  where ID = %1").arg(currentPhraseID));
	QSqlDatabase::database().commit();

	// -------------- update proximity by proximate phrases ---------------
	phraseThesaurus->request(model.data(model.index(currentRow, DICTIONARY_PHRASE)).toString());

	sortByProximity();
	jumpToID(currentPhraseID);     // keep highlighting
}

// thesaurus returns related phrases
void PageDictionary::onPhraseThesaurus(const QStringList& relatedPhrases)
{
	if(relatedPhrases.isEmpty())
		return;

	QSqlDatabase::database().transaction();
	QSqlQuery query;         // update proximity
	foreach(QString related, relatedPhrases) {
		query.exec(tr("update Dictionary set Proximity = \
						  (select Proximity from Dictionary where Phrase = \"%1\")+1 \
					  where Phrase = \"%1\"").arg(related));
	}

	// set itself the max proximity
	query.exec(tr("update Dictionary set Proximity = \
					  (select max(Proximity)+1 from Dictionary where ID <> %1) \
				  where ID = %1").arg(currentPhraseID));
	QSqlDatabase::database().commit();

	sortByProximity();
	jumpToID(currentPhraseID);    // keep highlighting
}

// thesaurus returns related tags
void PageDictionary::onTagThesaurus(const QStringList &relatedTags)
{
	if(relatedTags.isEmpty())
		return;

	QSqlDatabase::database().transaction();

	// get the IDs of relatedTags
	QSqlQuery query;
	QStringList tagIDs;
	foreach(QString tagName, relatedTags)
	{
		int tagID = getTagID("DictionaryTags", tagName);
		if(tagID > 0)
			tagIDs << QString::number(tagID);
	}

	// calculate proximity by proximate tags:
	// 1. count the # of all other phrases that have these tags
	// 2. associate the # with the phrases
	query.exec(tr("select Dictionary.ID, count(PhraseTag.Phrase) Proximity from Dictionary, PhraseTag \
				  where Tag in (%1) \
				  and ID != %2 and ID = PhraseTag.Phrase \
				  group by PhraseTag.Phrase").arg(tagIDs.join(",")).arg(currentPhraseID));

	// update proximity
	QSqlQuery subQuery;
	while(query.next()) {
		subQuery.exec(tr("update Dictionary set Proximity = \
						 (select Proximity from Dictionary where ID = %1) + %2 \
						 where ID = %1")
			.arg(query.value(0).toInt())
			.arg(query.value(1).toInt()));
	}

	// give itself the max proximity
	query.exec(tr("update Dictionary set Proximity = (select max(Proximity)+1 from Dictionary where ID <> %1) \
				  where ID = %1").arg(currentPhraseID));

	QSqlDatabase::database().commit();

	sortByProximity();
	jumpToID(currentPhraseID);    // keep highlighting
}

void PageDictionary::hideRelated()
{
	QSqlQuery query;
	query.exec(tr("update Dictionary set Proximity = 0"));   // reset proximity
}

void PageDictionary::sortByPhrase() {
	ui.tableView->sortByColumn(DICTIONARY_PHRASE, Qt::AscendingOrder);
}

void PageDictionary::sortByProximity() {
	ui.tableView->sortByColumn(DICTIONARY_PROXIMITY, Qt::DescendingOrder);
}
