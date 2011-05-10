#include "TagsWidget.h"
#include "Common.h"
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
		return;

	QAction actionAdd(tr("Add to paper"), this);
	QAction actionDel(tr("Delete"), this);
	connect(&actionAdd, SIGNAL(triggered()), this, SLOT(onAdd()));
	connect(&actionDel, SIGNAL(triggered()), this, SLOT(onDel()));
	QMenu menu(this);
	menu.addAction(&actionAdd);
	menu.addAction(&actionDel);
	menu.exec(event->globalPos());
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

void TagsWidget::onAdd()
{

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