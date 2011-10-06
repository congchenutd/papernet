#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include "ui_PagePapers.h"
#include "PaperModel.h"
#include "Page.h"

struct ImportResult;
class PaperDlg;

class PagePapers : public Page
{
	Q_OBJECT

public:
	PagePapers(QWidget* parent = 0);
	~PagePapers();
	void jumpToPaper(const QString& title);
	void saveGeometry();

	virtual void add();
	virtual void del();
	virtual void search(const QString& target);
	virtual void jumpToID(int id);

private slots:
	void onImport();
	void onCurrentRowChanged(const QModelIndex& idx);
	void onEditPaper();
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
	void onTagDoubleClicked(const QString& label);

	void onAddQuote();
	void onEditQuote(const QModelIndex& idx);
	void onDelQuotes();

signals:
	void tableValid(bool);

private:
	int  getPaperID(int row) const;
	void highLightTags();
	void hideRelated();
	void hideCoauthor();
	void mergeRecord(int row, const ImportResult& record);
	void insertRecord(const ImportResult& record);
	void updateQuotes();
	int  getQuoteID(int row) const;
	void loadSplitterSizes();
	void updateRecord(int row, const PaperDlg& dlg);
	void updateTags(const QStringList& tags);
	void setCurrentRow(int row);

private:
	Ui::PagePapersClass ui;

	PaperModel        modelPapers;
	QSqlQueryModel    modelQuotes;
	QDataWidgetMapper mapper;
	int               currentRow;
	int               currentPaperID;
	UserSetting*      setting;
};

#endif // PAGEPAPERS_H
