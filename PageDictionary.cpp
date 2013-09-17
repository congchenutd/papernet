#include "PageDictionary.h"
#include "Common.h"
#include "PhraseDlg.h"
#include "TagDlg.h"
#include "OptionDlg.h"
#include "Navigator.h"
#include <QMessageBox>
#include <QSqlQuery>

PageDictionary::PageDictionary(QWidget *parent)
	: Page(parent)
{
	ui.setupUi(this);
    _currentRow      = -1;
    _currentPhraseID = -1;

	resetPhrases();   // init model
    ui.tableView->setModel(&_model);
    ui.tableView->hideColumn(DICT_ID);
    ui.tableView->resizeColumnToContents(DICT_PHRASE);
    ui.tableView->sortByColumn(DICT_PHRASE, Qt::AscendingOrder);

	ui.widgetWordCloud->setTableNames("DictionaryTags", "PhraseTag", "Phrase");
	loadGeometry();

    connect(ui.tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection)));
	connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEdit()));
	connect(ui.tableView, SIGNAL(clicked      (QModelIndex)), this, SLOT(onClicked(QModelIndex)));

    connect(ui.widgetWordCloud, SIGNAL(filter(bool)), this, SLOT(onFilterByTags(bool)));
	connect(ui.widgetWordCloud, SIGNAL(newTag()),     this, SLOT(onAddTag()));
    connect(ui.widgetWordCloud, SIGNAL(addTag()),     this, SLOT(onAddTagsToPhrases()));
    connect(ui.widgetWordCloud, SIGNAL(removeTag()),  this, SLOT(onRemoveTagsFromPhrases()));
	connect(ui.widgetWordCloud, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));
    connect(ui.widgetRelated,   SIGNAL(doubleClicked(int)),     this, SLOT(onRelatedDoubleClicked(int)));
}

void PageDictionary::addRecord()
{
    PhraseDlg dlg(this);
	dlg.setWindowTitle(tr("Add Phrase"));
	if(dlg.exec() == QDialog::Accepted)
	{
		fetchAll(&_model);
        int lastRow = _model.rowCount();
        _currentPhraseID = getNextID("Dictionary", "ID");
        _model.insertRow(lastRow);
        _model.setData(_model.index(lastRow, DICT_ID),          _currentPhraseID);
        _model.setData(_model.index(lastRow, DICT_PHRASE),      dlg.getPhrase());
        _model.setData(_model.index(lastRow, DICT_EXPLANATION), dlg.getExplanation());
        _model.submitAll();
        updateTags(_currentPhraseID, dlg.getTags());
	}
}

void PageDictionary::delRecord()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tableView->selectionModel()->selectedRows();
		foreach(QModelIndex idx, idxList)
            delPhrase(rowToID(idx.row()));
        _model.select();
    }
}

void PageDictionary::editRecord() {
    onEdit();
}

void PageDictionary::onEdit()
{
    PhraseDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Phrase"));
    dlg.setPhrase     (_model.data(_model.index(_currentRow, DICT_PHRASE))     .toString());
    dlg.setExplanation(_model.data(_model.index(_currentRow, DICT_EXPLANATION)).toString());
    dlg.setTags       (getTagsOfPhrase(_currentPhraseID));
	if(dlg.exec() == QDialog::Accepted)
	{
        _model.setData(_model.index(_currentRow, DICT_PHRASE),      dlg.getPhrase());
        _model.setData(_model.index(_currentRow, DICT_EXPLANATION), dlg.getExplanation());
        _model.submitAll();
        updateTags(_currentPhraseID, dlg.getTags());
	}
}

// recreate the relations to tags
void PageDictionary::updateTags(int phraseID, const QStringList& tags)
{
    // delete all the old relations
    QSqlQuery query;
    query.exec(tr("delete from PhraseTag where Phrase = %1").arg(phraseID));

    // create new relations
	foreach(QString tag, tags)
	{
		if(tag.isEmpty())
			continue;
		int tagID = getTagID("DictionaryTags", tag);
        if(tagID < 0)     // a new tag
		{
			tagID = getNextID("DictionaryTags", "ID");
			ui.widgetWordCloud->addTag(tagID, tag);
		}
        ui.widgetWordCloud->addTagToItem(tagID, phraseID);
	}
	highLightTags();
}

void PageDictionary::onSelectionChanged(const QItemSelection& selected)
{
    if(!selected.isEmpty())
    {
        _currentRow      = selected.indexes().front().row();
        _currentPhraseID = rowToID(_currentRow);
        highLightTags();
        ui.widgetRelated->setCentralPhraseID(_currentPhraseID);
    }
    emit selectionValid(!selected.isEmpty());
}

void PageDictionary::onClicked(const QModelIndex& idx) {
    Navigator::getInstance()->addFootStep(this, rowToID(idx.row()));   // navigation
}

// new tag
void PageDictionary::onAddTag()
{
	TagDlg dlg("DictionaryTags", this);
	if(dlg.exec() == QDialog::Accepted)
	{
		int tagID = getNextID("DictionaryTags", "ID");
		ui.widgetWordCloud->addTag(tagID, dlg.getText());
        ui.widgetWordCloud->addTagToItem(tagID, _currentPhraseID);
		highLightTags();
	}
}

// add selected tags to selected phrases
void PageDictionary::onAddTagsToPhrases()
{
    QModelIndexList rows = ui.tableView->selectionModel()->selectedRows(DICT_ID);
    foreach(const QModelIndex& idx, rows)
	{
        int phraseID = _model.data(idx).toInt();
		QList<WordLabel*> tags = ui.widgetWordCloud->getSelected();
		foreach(WordLabel* tag, tags)
			ui.widgetWordCloud->addTagToItem(getTagID("DictionaryTags", tag->text()),
											 phraseID);
	}
	highLightTags();
}

// del selected tags from selected phrases
void PageDictionary::onRemoveTagsFromPhrases()
{
	QModelIndexList rows = ui.tableView->selectionModel()->selectedRows(PAPER_ID);
	foreach(QModelIndex idx, rows)
	{
        int phraseID = _model.data(idx).toInt();
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
        _model.setFilter(
			tr("ID in (select Phrase from PhraseTag where Tag in (%1))").arg(tagIDs.join(",")));
	else
	{
		dropTempView();
		QSqlQuery query;    // create a temp table for selected tags id
        query.exec(tr("create view SelectedTags as \
                         select * from DictionaryTags\
                         where ID in (%1)").arg(tagIDs.join(",")));

		// select phrases that contain all the selected tags
        _model.setFilter("not exists \
						 (select * from SelectedTags where \
						 not exists \
							 (select * from PhraseTag where \
							 phrase=Dictionary.ID and Tag=SelectedTags.ID))");
	}
}

void PageDictionary::highLightTags() {
    ui.widgetWordCloud->highLight(getTagsOfPhrase(_currentPhraseID));
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

void PageDictionary::resetPhrases()
{
    _model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    _model.setTable("Dictionary");
    _model.select();
	fetchAll(&_model);
    jumpToID(_currentPhraseID);
    ui.tableView->sortByColumn(DICT_PHRASE, Qt::AscendingOrder);
}

void PageDictionary::jumpToID(int id)
{
    fetchAll(&_model);
    int row = idToRow(&_model, DICT_ID, id);
	if(row > -1)
	{
        _currentRow = row;
        ui.tableView->selectRow(_currentRow);  // will trigger onSelectionChanged()
		ui.tableView->setFocus();
        ui.tableView->scrollTo(_model.index(row, DICT_PHRASE));
    }
}

void PageDictionary::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
		resetPhrases();
	else
        onFilterByTags();
}

void PageDictionary::onRelatedDoubleClicked(int phraseID)
{
    _model.setTable("Dictionary");
    _model.select();
    jumpToID(phraseID);
    Navigator::getInstance()->addFootStep(this, phraseID);
}

void PageDictionary::search(const QString& target)
{
    _model.setFilter(tr("Phrase      like \"%%1%\" or \
                         Explanation like \"%%1%\" ").arg(target));

	// highlight tags
	ui.widgetWordCloud->search(target);
}

int PageDictionary::rowToID(int row) const {
    return _model.data(_model.index(row, DICT_ID)).toInt();
}
