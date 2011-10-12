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
class Thesaurus;

class PagePapers : public Page
{
	Q_OBJECT

public:
	PagePapers(QWidget* parent = 0);
	~PagePapers();
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
	void onFilterPapers(bool AND = false);
	void onTagDoubleClicked(const QString& label);

	void onAddQuote();
	void onEditQuote(const QModelIndex& idx);
	void onDelQuotes();

	void onThesaurus(const QStringList& relatedTags);

signals:
	void tableValid(bool);

private:
	int  getPaperID(int row) const;
	void hideRelated();
	void hideCoauthor();
	void mergeRecord(int row, const ImportResult& record);
	void insertRecord(const ImportResult& record);
	void updateRecord(int row, const PaperDlg& dlg);
	void updateQuotes();
	void updateTags(const QStringList& tags);
	void highLightTags();
	int  getQuoteID(int row) const;
	void loadSplitterSizes();

private:
	Ui::PagePapersClass ui;

	PaperModel        modelPapers;
	QSqlQueryModel    modelQuotes;
	QDataWidgetMapper mapper;
	int               currentRow;
	int               currentPaperID;
	UserSetting*      setting;
	Thesaurus*        thesaurus;
};

#endif // PAGEPAPERS_H
