#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QSqlTableModel>
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
	void onClicked();
	void onResetPapers();
	void setPaperRead();

    void onRelatedDoubleClicked(int paperID);
	void onQuoteDoubleClicked(int quoteID);

	void onNewTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();
	void onFilterPapersByTags(bool AND = false);
	void onTagDoubleClicked(const QString& label);

	void onPrintMe(bool print);   // two built-in tags
	void onBookmark (bool readMe);
	void onAddQuote();
	void onAddPDF();
	void onReadPDF();

signals:
	void tableValid(bool);

private:
	int  rowToID(int row) const;
	void insertRecord(const PaperRecord& record);
	void mergeRecord (int row, const PaperRecord& record);
	void updateRecord(int row, const PaperRecord& record);
	void updateTags(const QStringList& tags);
	void highLightTags();
	void loadGeometry();
	void reloadAttachments();
	void attachNewTag(const QString& tagName);   // create a tag, and add it to current paper

private:
	Ui::PagePapersClass ui;

	PaperModel        modelPapers;
	QDataWidgetMapper mapper;
	int               currentRow;
	int               currentPaperID;
	UserSetting*      setting;
};

#endif // PAGEPAPERS_H
