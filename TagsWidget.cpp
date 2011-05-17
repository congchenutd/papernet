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
	QAction actionNewTag(tr("New Tag"), this);
	QAction actionFilter(tr("Show Tagged Items"), this);
	QAction actionAdd   (tr("Add"), this);
	QAction actionRemove(tr("Remove"), this);
	QAction actionRename(tr("Rename"), this);
	QAction actionDel   (tr("Delete"), this);
	connect(&actionNewTag, SIGNAL(triggered()), this, SIGNAL(newTag()));
	connect(&actionFilter, SIGNAL(triggered()), this, SIGNAL(filter()));
	connect(&actionAdd,    SIGNAL(triggered()), this, SIGNAL(addTag()));
	connect(&actionRemove, SIGNAL(triggered()), this, SIGNAL(removeTag()));
	connect(&actionRename, SIGNAL(triggered()), this, SLOT(onRename()));
	connect(&actionDel,    SIGNAL(triggered()), this, SLOT(onDel()));
	menu.addAction(&actionNewTag);
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

void TagsWidget::updateSizes()
{
	QSqlQuery query;
	query.exec(tr("select Name, Size from %1").arg(tagTableName));
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

void TagsWidget::addTagToPaper(int tagID, int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("insert into %1 values(%2, %3)")
					.arg(relationTableName).arg(paperID).arg(tagID));
	updatePaperTagSize(tagID);   // recalculate tag size
	updateSizes();
}

void TagsWidget::removeTagFromPaper(int tagID, int paperID)
{
	QSqlQuery query;
	query.exec(QObject::tr("delete from %1 where Paper=%2 and Tag=%3")
					.arg(relationTableName).arg(paperID).arg(tagID));
	updatePaperTagSize(tagID);   // recalculate tag size
	updateSizes();
}

void TagsWidget::setTableNames(const QString& tagName, const QString& relationName)
{
	tagTableName      = tagName;
	relationTableName = relationName;
}