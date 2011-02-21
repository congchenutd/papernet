#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include "ui_PagePapers.h"
#include "PaperModel.h"

class QDataWidgetMapper;
struct ImportResult;

class PagePapers : public QWidget
{
	Q_OBJECT

public:
	PagePapers(QWidget *parent = 0);
	~PagePapers();

protected:
	virtual void resizeEvent(QResizeEvent*);

private slots:
	void onCurrentRowPapersChanged(const QModelIndex& idx);
	void onAddPaper();
	void onEditPaper();
	void onDelPaper();
	void onImport();
	void onSearch(const QString& target);
	void onSubmitPaper();
	void onClicked(const QModelIndex& idx);
	void onShowRelated();
	void onShowCoauthored();
	void onAddSnippet();
	void onResetPapers();

	void onCurrentRowAllTagsChanged();
	void onAddTag();
	void onDelTag();
	void onEditTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();

	void onCurrentRowTagsChanged();
	void onFilter(bool enabled);

	void onShowTags(bool show);

private:
	int getPaperID (int row) const;
	int getTagID   (int row) const;
	int getAllTagID(int row) const;
	void selectID(int id);
	int idToRow(int id) const;
	void updateTags();
	void filterPapers();
	void resetAllTags();
	bool isFiltered() const;
	void hideRelated();
	void hideCoauthor();
    void mergeRecord(int row, const ImportResult& record);
    void insertRecord(const ImportResult& record);

public:
	enum {PAPER_ID, PAPER_READ, PAPER_TAGGED, PAPER_ATTACHED, PAPER_TITLE, 
		  PAPER_AUTHORS, PAPER_YEAR, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_NOTE, 
		  PAPER_PROXIMITY, PAPER_COAUTHOR, PAPER_ADDEDTIME};
	enum {TAG_ID, TAG_NAME};

private:
	Ui::PagePapersClass ui;

	PaperModel modelPapers;
	QSqlTableModel modelAllTags;
	QSqlQueryModel modelTags;
	int currentRowPapers;
	int currentRowTags;
	int currentPaperID;
	QDataWidgetMapper* mapper;
};

#endif // PAGEPAPERS_H
