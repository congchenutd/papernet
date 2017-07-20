#include "TagsWidget.h"
#include "Common.h"
#include "TagDlg.h"
#include "OptionDlg.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QSqlQuery>
#include <QSqlError>

TagsWidget::TagsWidget(QWidget* parent) : WordCloudWidget(parent)
{
	int minFont = UserSetting::getInstance()->getFont().pointSize();
	setSizeRange(minFont, minFont*2);

	actionNewTag   = new QAction(QIcon(":/Images/AddTag.png"),     tr("New tag"), this);
	actionFilter   = new QAction(QIcon(":/Images/ShowTagged.png"), tr("Show tagged (OR)"), this);
	actionAdd      = new QAction(QIcon(":/Images/Backward.png"),   tr("Add"),      this);
	actionRemove   = new QAction(QIcon(":/Images/Forward.png"),    tr("Remove"), this);
	actionRename   = new QAction(QIcon(":/Images/EditTag.png"),    tr("Rename"), this);
	actionDel      = new QAction(QIcon(":/Images/DelTag.png"),     tr("Delete tag"), this);

	connect(actionNewTag,   SIGNAL(triggered()), this, SIGNAL(newTag()));
	connect(actionAdd,      SIGNAL(triggered()), this, SIGNAL(addTag()));
	connect(actionRemove,   SIGNAL(triggered()), this, SIGNAL(removeTag()));
	connect(actionFilter,   SIGNAL(triggered()), this, SLOT(onFilter()));
	connect(actionRename,   SIGNAL(triggered()), this, SLOT(onRename()));
	connect(actionDel,      SIGNAL(triggered()), this, SLOT(onDel()));
}

void TagsWidget::contextMenuEvent(QContextMenuEvent* event)
{
	TagsWidgetMenu menu(actionFilter, this);
	menu.addAction(actionNewTag);
	if(childAt(event->pos()) != 0)         // click on a tag
	{
        menu.addSeparator();
		menu.setAND(controlIsPressed());   // init actionFilter
		menu.addAction(actionFilter);
		menu.addAction(actionAdd);
		menu.addAction(actionRemove);
		menu.addAction(actionRename);
		menu.addAction(actionDel);
	}
	menu.exec(event->globalPos());
}

void TagsWidget::onDel()
{
	QList<WordLabel*> selected = getSelected();
	foreach(WordLabel* word, selected)       // delete all selected
	{
		removeWord(word);                    // from the cloud
		delTag(tagTableName, word->text());  // from db
	}
}

// rebuild the cloud
void TagsWidget::rebuild()
{
	QSqlQuery query;
	query.exec(tr("select Name, Size from %1 order by Name").arg(tagTableName));
	while(query.next())
	{
		QString word = query.value(0).toString();
        if (word.toLower() == "readme")
            continue;

        int size = query.value(1).toInt();
        if (WordLabel* label = findWord(word))
			label->setSize(size);
		else
			addWord(word, size);
	}
	normalizeSizes();  // calculate the display sizes
}

void TagsWidget::onRename()
{
	QList<WordLabel*> selected = getSelected();
	if(selected.size() > 1)
		return;

	WordLabel* word = selected.front();                    // one at a time
	QString oldName = word->text();
	TagDlg dlg(tagTableName, this);
	dlg.setWindowTitle(tr("Edit Tag"));
	dlg.setText(oldName);
	if(dlg.exec() == QDialog::Accepted)
	{
		renameWord(word, dlg.getText());                   // update the cloud
		renameTag(tagTableName, oldName, dlg.getText());   // update the db
	}
}

void TagsWidget::addTag(int id, const QString& text)
{
	if(text.isEmpty())
		return;

	// add to the db
	QSqlQuery query;
	query.prepare(tr("insert into %1 values(:id, :text, 0)").arg(tagTableName));
	query.bindValue(":id",   id);
	query.bindValue(":text", text);
	query.exec();

	// add to the cloud
	addWord(text);
	normalizeSizes();
}

// associate the tag with the item
void TagsWidget::addTagToItem(int tagID, int itemID)
{
	QSqlQuery query;
	query.exec(tr("insert into %1 values(%2, %3)")
			   .arg(relationTableName).arg(itemID).arg(tagID));
    qDebug() << query.lastError().text();
    updateTagSize(tagID);                // recalculate tag size
    highLight(getTagsOfPaper(itemID));   // highlight the affected paper
}


// deassociate the tag with the item
void TagsWidget::removeTagFromItem(int tagID, int itemID)
{
	if(tagID < 0 || itemID < 0)
		return;

	QSqlQuery query;
	query.exec(tr("delete from %1 where %2=%3 and Tag=%4")
			   .arg(relationTableName)
			   .arg(relationSectionName)
			   .arg(itemID)
			   .arg(tagID));
    updateTagSize(tagID);                 // recalculate tag size
    highLight(getTagsOfPaper(itemID));    // highlight the affected paper
}

// init
void TagsWidget::setTableNames(const QString& tagName, const QString& relationName, const QString& relationSection)
{
	tagTableName        = tagName;
	relationTableName   = relationName;
	relationSectionName = relationSection;
	rebuild();
}

void TagsWidget::updateTagSize(int tagID)
{
	if(tagID < 0)
		return;

	// calculate the raw size of a tag
	QSqlQuery query;
	query.exec(tr("select count(*) from %1 group by Tag having Tag = %2")
			   .arg(relationTableName).arg(tagID));
	int size = query.next() ? query.value(0).toInt() : 0;
	query.exec(tr("update %1 set Size=%2 where ID=%3")
			   .arg(tagTableName).arg(size).arg(tagID));
	rebuild();
}

void TagsWidget::onFilter() {
	emit filter(actionFilter->text() == tr("Show tagged (AND)"));
}


/////////////////////////////////////////////////////////////
TagsWidgetMenu::TagsWidgetMenu(QAction* action, QWidget* parent)
	: QMenu(parent), actionFilter(action) {}

void TagsWidgetMenu::keyPressEvent(QKeyEvent* event)
{
	setAND(event->modifiers() == Qt::ControlModifier);   // ctrl pressed on not
	QMenu::keyPressEvent(event);
}

// show AND/OR on the filter action
void TagsWidgetMenu::setAND(bool AND) {
	actionFilter->setText(AND ? tr("Show tagged (AND)") : tr("Show tagged (OR)"));
}
