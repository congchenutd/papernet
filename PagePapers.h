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
	void onCurrentRowChanged(const QModelIndex& idx);
	void onEditPaper();
	void onSearch(const QString& target);
	void onFullTextSearch(const QString& target);
	void onSubmitPaper();
	void onClicked(const QModelIndex& idx);
	void onShowRelated();
	void onShowCoauthored();
	void onResetPapers();

	void onAddTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();
	void onFilterPapers();

	void onAddSnippet();
	void onEditSnippet(const QModelIndex& idx);
	void onDelSnippets();

signals:
	void tableValid(bool);

private:
	int  getPaperID(int row) const;
	int  idToRow   (int id)  const;
	void selectID(int id);
	void updateTags();
	void hideRelated();
	void hideCoauthor();
	void mergeRecord(int row, const ImportResult& record);
	void insertRecord(const ImportResult& record);
	void updateSnippets();
	int  getSnippetID(int row) const;
	void loadSplitterSizes();

private:
	Ui::PagePapersClass ui;

	PaperModel        modelPapers;
	QSqlQueryModel    modelSnippets;
	QDataWidgetMapper mapper;
	int               currentRow;
	int               currentPaperID;
	UserSetting*      setting;
};

#endif // PAGEPAPERS_H
