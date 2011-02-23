#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include "ui_PagePapers.h"
#include "PaperModel.h"

struct ImportResult;

class PagePapers : public QWidget
{
	Q_OBJECT

public:
	PagePapers(QWidget *parent = 0);
	~PagePapers();
	void jumpToPaper(const QString& title);
	void saveSectionSizes();
	void saveSplitterSizes();

private slots:
	void onAddPaper();
	void onDelPaper();
	void onImport();
	void onCurrentRowPapersChanged(const QModelIndex& idx);
	void onEditPaper();
	void onSearch(const QString& target);
	void onFullTextSearch(const QString& target);
	void onSubmitPaper();
	void onClicked(const QModelIndex& idx);
	void onShowRelated();
	void onShowCoauthored();
	void onResetPapers();

	void onCurrentRowAllTagsChanged();
	void onAddTag();
	void onDelTag();
	void onEditTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();

	void onCurrentRowTagsChanged();
	void onFilter(bool enabled);

	void onAddSnippet();
	void onEditSnippet(const QModelIndex& idx);
	void onDelSnippets();

signals:
	void tableValid(bool);

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
	void updateSnippets();
	int  getSnippetID(const QModelIndex& idx) const;
	void loadSplitterSizes();

private:
	Ui::PagePapersClass ui;

	PaperModel modelPapers;
	QSqlTableModel modelAllTags;
	QSqlQueryModel modelTags;
	int currentRowPapers;
	int currentRowTags;
	int currentPaperID;
	QDataWidgetMapper mapper;
	QSqlQueryModel modelSnippets;
	UserSetting* setting;
};

#endif // PAGEPAPERS_H
