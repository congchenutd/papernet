#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "../WordCloud/WordCloudWidget.h"

class TagsWidget : public WordCloudWidget
{
	Q_OBJECT

public:
	TagsWidget(QWidget* parent = 0);
	void setTableNames(const QString& tagName, const QString& relationName, const QString& relationSection);
	void updateSizes();
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

signals:
	void filter();
	void unfilter();
	void removeTag();
	void addTag();
	void newTag();

private:
	QString tagTableName;
	QString relationTableName;
	QString relationSectionName;
};

#endif // TAGSWIDGET_H
