#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QSqlTableModel>
#include "ui_PagePapers.h"
#include "PaperModel.h"
#include "Page.h"
#include "Reference.h"

class UserSetting;
class PaperWidgetMapper;

class PagePapers : public Page
{
	Q_OBJECT

public:
	PagePapers(QWidget* parent = 0);
    void saveGeometry();    // called by mainwindow, closeEvent() only works for top window

    void addRecord();
    void delRecord();
    void editRecord();
    void search(const QString& target);
    void jumpToID(int id);
    void reset() { resetModel(); }

    void importFromFiles(const QStringList& filePaths);

public slots:
    void jumpToCurrent() { jumpToID(_currentID); }

private slots:
    void onSelectionChanged(const QItemSelection& selected);
	void onEditPaper();
	void onFullTextSearch(const QString& target);
	void onClicked();
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

    // handle menu signals
    void onPrintMe (bool print);    // two built-in tags
	void onBookmark(bool readMe);
	void onAddQuote();
    void onAddPDF();                // add pdf to current paper
	void onReadPDF();

signals:
    void selectionValid(bool);
    void hasPDF(bool);

private:
    void resetModel();              // just reset the model
	int  rowToID(int row) const;
    void updateQuotes();
    void setPaperRead();
    void recreateTagsRelations(const QStringList& tags);
	void highLightTags();
	void loadGeometry();
    void attachNewTag(const QString& tagName);   // create a tag and add it to current paper

    void submit();
    void insertReference(const Reference& ref);                // insert or replace
    void updateRefByRow(int row, const Reference& ref);        // update a row by the ref
    void updateRefByID (int id,  const Reference& ref);
    void importReferences(const QList<Reference>& references); // review and insert
    QList<Reference> parseContent(const QString& content);     // using all possible specs' parsers

    Reference exportRef(int id) const;
    Reference recordToRef(const QSqlRecord& record) const;
    QString toString(const QModelIndexList& idxList, const QString& extension);  // export the selected

private:
	Ui::PagePapersClass ui;

    PaperModel   _model;
    PaperWidgetMapper* _mapper;
    int          _currentRow;
    int          _currentID;
    UserSetting* _setting;
};

#endif // PAGEPAPERS_H
