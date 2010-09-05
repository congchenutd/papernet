#ifndef TAGLIST_H
#define TAGLIST_H

#include <QWidget>
#include <QtSql>
#include "ui_TagList.h"

class TagList : public QWidget
{
	Q_OBJECT

public:
	TagList(QWidget *parent = 0);

private slots:
	void onAddTag();
	void onEditTag();
	void onDelTag();
	void onCurrentRowChanged(const QModelIndex& idx);
	void onAddPaper();
	void onDelPaper();
	void onEnableDelPapers(const QModelIndex& idx);

private:
	int getCurrentTagID() const;
	void updatePapers();
	void updateRelatedTags();

public:
	enum {TAG_ID, TAG_NAME};

private:
	Ui::TagListClass ui;

	QSqlTableModel modelTags;
	QSqlQueryModel modelPapers;
	QSqlQueryModel modelRelatedTags;
	int currentRow;
};

#endif // TAGLIST_H
