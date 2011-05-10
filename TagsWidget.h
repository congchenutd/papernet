#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "../WordCloud/WordCloudWidget.h"

class QSqlTableModel;

class TagsWidget : public WordCloudWidget
{
	Q_OBJECT

public:
	TagsWidget(QWidget* parent = 0);
	void updateSizes();

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
	QSqlTableModel* model;
};

#endif // TAGSWIDGET_H
