#include "TagsWidget.h"
#include "Common.h"
#include "AddTagDlg.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QSqlTableModel>

TagsWidget::TagsWidget(QWidget* parent) : WordCloudWidget(parent)
{
	model = new QSqlTableModel(this);
	model->setTable("Tags");
}

void TagsWidget::contextMenuEvent(QContextMenuEvent* event)
{
	if(childAt(event->pos()) == 0)
	{
		QAction actionNewTag(tr("New Tag"), this);
		connect(&actionNewTag, SIGNAL(triggered()), this, SIGNAL(newTag()));
		QMenu menu(this);
		menu.addAction(&actionNewTag);
		menu.exec(event->globalPos());
	}
	else
	{
		QAction actionAdd   (tr("Add to Paper"), this);
		QAction actionRemove(tr("Remove from Paper"), this);
		QAction actionRename(tr("Rename Tag"), this);
		QAction actionDel   (tr("Delete Tag"), this);
		connect(&actionAdd,    SIGNAL(triggered()), this, SIGNAL(addTag()));
		connect(&actionRemove, SIGNAL(triggered()), this, SIGNAL(removeTag()));
		connect(&actionRename, SIGNAL(triggered()), this, SLOT(onRename()));
		connect(&actionDel,    SIGNAL(triggered()), this, SLOT(onDel()));
		QMenu menu(this);
		menu.addAction(&actionAdd);
		menu.addAction(&actionRemove);
		menu.addAction(&actionRename);
		menu.addAction(&actionDel);
		menu.exec(event->globalPos());
	}
}

void TagsWidget::onDel()
{
	QList<WordLabel*> selected = getSelected();
	foreach(WordLabel* word, selected)   // del selected
	{
		removeWord(word);
		delTag(word->text());
	}
}

void TagsWidget::updateSizes()
{
	model->select();
	for(int i=0; i<model->rowCount(); ++i)
	{
		QString word = model->data(model->index(i, TAG_NAME)).toString();
		int     size = model->data(model->index(i, TAG_SIZE)).toInt();
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
	AddTagDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Tag"));
	WordLabel* word = selected.front();
	QString oldName = word->text();
	dlg.setText(oldName);
	if(dlg.exec() == QDialog::Accepted && !dlg.getText().isEmpty())
	{
		word->setText(dlg.getText());
		renameTag(oldName, dlg.getText());
	}
}
