#include "TagsWidget.h"
#include "Common.h"
#include "AddTagDlg.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QSqlQuery>

TagsWidget::TagsWidget(QWidget* parent) : WordCloudWidget(parent)
{}

void TagsWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);
	QAction actionNewTag  (tr("New Tag"),     this);
	QAction actionFilter  (tr("Show Tagged"), this);
	QAction actionUnFilter(tr("Show All"),    this);
	QAction actionAdd     (tr("Add"),         this);
	QAction actionRemove  (tr("Remove"),      this);
	QAction actionRename  (tr("Rename"),      this);
	QAction actionDel     (tr("Delete"),      this);
	connect(&actionNewTag,   SIGNAL(triggered()), this, SIGNAL(newTag()));
	connect(&actionFilter,   SIGNAL(triggered()), this, SIGNAL(filter()));
	connect(&actionUnFilter, SIGNAL(triggered()), this, SIGNAL(unfilter()));
	connect(&actionAdd,      SIGNAL(triggered()), this, SIGNAL(addTag()));
	connect(&actionRemove,   SIGNAL(triggered()), this, SIGNAL(removeTag()));
	connect(&actionRename,   SIGNAL(triggered()), this, SLOT(onRename()));
	connect(&actionDel,      SIGNAL(triggered()), this, SLOT(onDel()));
	menu.addAction(&actionNewTag);
	menu.addAction(&actionUnFilter);
	if(childAt(event->pos()) != 0)
	{
		menu.addAction(&actionFilter);
		menu.addAction(&actionAdd);
		menu.addAction(&actionRemove);
		menu.addAction(&actionRename);
		menu.addAction(&actionDel);
	}
	menu.exec(event->globalPos());
}

void TagsWidget::onDel()
{
	QList<WordLabel*> selected = getSelected();
	foreach(WordLabel* word, selected)   // del selected
	{
		removeWord(word);
		delTag(tagTableName, word->text());  // db stuff
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
		int     size = query.value(1).toInt();
		if(WordLabel* label = findWord(word))
			label->setSize(size);
		else
			addWord(word, size);
	}
	normalizeSizes();
}

void TagsWidget::onRename()
{
	QList<WordLabel*> selected = getSelected();
	if(selected.size() > 1)
		return;
	AddTagDlg dlg(tagTableName, this);
	dlg.setWindowTitle(tr("Edit Tag"));
	WordLabel* word = selected.front();
	QString oldName = word->text();
	dlg.setText(oldName);
	if(dlg.exec() == QDialog::Accepted && !dlg.getText().isEmpty())
	{
		word->setText(dlg.getText());
		renameTag(tagTableName, oldName, dlg.getText());   // db
	}
}

void TagsWidget::addTag(int id, const QString& text)
{
	// add to db
	QSqlQuery query;
	query.exec(tr("insert into %1 values(%2, \"%3\", 0)")
			   .arg(tagTableName).arg(id).arg(text));

	addWord(text, 20);
	normalizeSizes();
}

void TagsWidget::addTagToItem(int tagID, int itemID)
{
	QSqlQuery query;
	query.exec(tr("insert into %1 values(%2, %3)")
			   .arg(relationTableName).arg(itemID).arg(tagID));
	updateTagSize(tagID);   // recalculate tag size
}

void TagsWidget::removeTagFromItem(int tagID, int itemID)
{
	QSqlQuery query;
	query.exec(tr("delete from %1 where %2=%3 and Tag=%4")
			   .arg(relationTableName)
			   .arg(relationSectionName)
			   .arg(itemID)
			   .arg(tagID));
	updateTagSize(tagID);   // recalculate tag size
}

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

	QSqlQuery query;
	query.exec(tr("select count(*) from %1 group by Tag having Tag = %2")
			   .arg(relationTableName).arg(tagID));
	int size = query.next() ? query.value(0).toInt() : 0;
	query.exec(tr("update %1 set Size=%2 where ID=%3")
			   .arg(tagTableName).arg(size).arg(tagID));
	rebuild();
}
