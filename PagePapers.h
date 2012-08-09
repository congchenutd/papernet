#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include "ui_PagePapers.h"
#include "PaperModel.h"
#include "Page.h"
#include "Reference.h"

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

    void importFromFiles(const QStringList& filePaths);

private slots:
    void onSelectionChanged(const QItemSelection& selected);
	void onEditPaper();
	void onFullTextSearch(const QString& target);
	void onSubmitPaper();
	void onClicked();
	void onResetPapers();
	void onImport();
	void onExport();

    // related and quotes
    void onRelatedDoubleClicked(int paperID);
    void onQuoteDoubleClicked  (int quoteID);

    // tags
	void onNewTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();
	void onFilterPapersByTags(bool AND = false);
	void onTagDoubleClicked(const QString& label);

    // menu signals
    void onPrintMe  (bool print);   // two built-in tags
	void onBookmark (bool readMe);

	void onAddQuote();
	void onAddPDF();
	void onReadPDF();

signals:
    void selectionValid(bool);
    void hasPDF(bool);

private:
	int  rowToID(int row) const;
    void setPaperRead();
	void updateTags(const QStringList& tags);
	void highLightTags();
	void loadGeometry();
	void reloadAttachments();
	void attachNewTag(const QString& tagName);   // create and add a tag to current paper

    void insertReference(const Reference& ref);
    void updateReference(int row, const Reference& ref);
    void importReferences(const QList<Reference>& references);
    void importPDF(const QString& pdfPath);
	Reference exportReference(int row) const;   // row -> Reference

private:
	Ui::PagePapersClass ui;

	PaperModel   model;
	int          currentRow;
	int          currentPaperID;
	UserSetting* setting;
};

#endif // PAGEPAPERS_H
