#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "../WordCloud/WordCloudWidget.h"

class TagsWidget : public WordCloudWidget
{
	Q_OBJECT

public:
	TagsWidget(QWidget* parent = 0);
	void setTableNames(const QString& tagName, const QString& relationName);
	void updateSizes();
	void addTag(int id, const QString& text);
	void addTagToPaper(int tagID, int paperID);
	void removeTagFromPaper(int tagID, int paperID);

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void onDel();
	void onRename();

signals:
	void filter();
	void removeTag();
	void addTag();
	void newTag();

private:
	QString tagTableName;
	QString relationTableName;
};

#endif // TAGSWIDGET_H
