#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "../WordCloud/WordCloudWidget.h"
#include <QMenu>

class TagsWidget : public WordCloudWidget
{
	Q_OBJECT

public:
	TagsWidget(QWidget* parent = 0);
	void setTableNames(const QString& tagName, const QString& relationName, const QString& relationSection);
	void rebuild();
	void addTag(int id, const QString& text);
	void addTagToItem     (int tagID, int itemID);
	void removeTagFromItem(int tagID, int itemID);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

private:
	void updateTagSize(int tagID);

private slots:
	void onDel();
	void onRename();
	void onFilter();

signals:
	void filter(bool controlPressed);
	void unfilter();
	void removeTag();
	void addTag();
	void newTag();

private:
	QString tagTableName;
	QString relationTableName;
	QString relationSectionName;

	QAction* actionNewTag;
	QAction* actionFilter;
	QAction* actionUnFilter;
	QAction* actionAdd;
	QAction* actionRemove;
	QAction* actionRename;
	QAction* actionDel;
};

class TagsWidgetMenu : public QMenu
{
public:
	TagsWidgetMenu(QAction* action, QWidget* parent = 0);
	void setAND(bool AND);

protected:
	virtual void keyPressEvent(QKeyEvent*);

private:
	QAction* actionFilter;
};

#endif // TAGSWIDGET_H
