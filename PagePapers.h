#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include "ui_PagePapers.h"
#include "PaperModel.h"
#include "Page.h"

struct PaperRecord;
class PaperDlg;
class Thesaurus;

class PagePapers : public Page
{
	Q_OBJECT

public:
	PagePapers(QWidget* parent = 0);
	void saveGeometry();

	virtual void add();
	virtual void del();
	virtual void search(const QString& target);
	virtual void jumpToID(int id);
	virtual void jumpToCurrent() { jumpToID(currentPaperID); }
	virtual void reset() { onResetPapers(); }

private slots:
	void onImport();
	void onCurrentRowChanged(const QModelIndex& idx);
	void onEditPaper();
	void onFullTextSearch(const QString& target);
	void onSubmitPaper();
	void onClicked(const QModelIndex& idx);
	void onResetPapers();
	void setPaperRead();
	void onRelatedPaperClicked(int paperID);

	void onAddTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();
	void onFilterPapers(bool AND = false);
	void onTagDoubleClicked(const QString& label);
	void onPrintMe(bool print);   // two built-in tags
	void onReadMe (bool readMe);

	void onAddQuote();
	void onEditQuote(const QModelIndex& idx);
	void onDelQuotes();

signals:
	void tableValid(bool);

private:
	int  getPaperID(int row) const;
	void insertRecord(const PaperRecord& record);
	void mergeRecord (int row, const PaperRecord& record);
	void updateRecord(int row, const PaperRecord& record);
	void updateQuotes();
	void updateTags(const QStringList& tags);
	void highLightTags();
	int  getQuoteID(int row) const;
	void loadSplitterSizes();
	void reloadAttachments();
	void sortByTitle();
	void attachNewTag(const QString& tagName);   // create a tag, and add it to current paper

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
