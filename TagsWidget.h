#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "../WordCloud/WordCloudWidget.h"
#include <QMenu>

// A widget of Tags based on WordCloud
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

private slots:                            // for menu actions
	void onDel();
	void onRename();
	void onFilter();

signals:                                  // for menu actions
	void filter(bool controlPressed);
	void unfilter();
	void removeTag();
	void addTag();
	void newTag();

private:
	QString tagTableName;           // (Paper)Tags, or DictionaryTags
	QString relationTableName;      // PaperTag, or PhraseTag
	QString relationSectionName;    // section name in the relation table, Paper, or Phrase

	QAction* actionNewTag;    // for the menu
	QAction* actionFilter;
	QAction* actionUnFilter;
	QAction* actionAdd;
	QAction* actionRemove;
	QAction* actionRename;
	QAction* actionDel;
};

// The menu used by TagsWidget
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
