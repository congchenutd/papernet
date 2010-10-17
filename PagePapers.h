#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include "ui_PagePapers.h"
//#include "ModelPapers.h"

class QDataWidgetMapper;

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
	void onShowSearch(bool enable);
	void onSearch(const QString& target);
	void onCancelSearch();
	void onSetPDF();
	void onReadPDF();
	void onSubmitPaper();

	void onCurrentRowAllTagsChanged();
	void onAddTag();
	void onDelTag();
	void onEditTag();
	void onAddTagToPaper();
	void onDelTagFromPaper();

	void onCurrentRowTagsChanged();
	void onFilter(bool enabled);

private:
	int getPaperID (int row) const;
	int getTagID   (int row) const;
	int getAllTagID(int row) const;
	int getCurrentPaperID() const;
	void selectID(int id);
	void import(const QString& fileName, const QString& firstHead, 
				const QString& titleHead, const QString& authorHead, 
				const QStringList& journalHeads, const QString& delimiter, 
				const QString& abstractHead = "NO_SUCH_HEAD");
	QString trimHead(const QString& line, const QString& delimiter) const;
	int idToRow(int id) const;
	QString getCurrentPDFPath() const;
	QString makePDFFileName(const QString& title) const;
	void updateTags();
	void updatePapers();
	void updateRelatedPapers();
	void resetPapers();
	void resetAllTags();
	bool isFiltered() const;

private:
	enum {PAPER_ID, PAPER_TITLE, PAPER_AUTHORS, PAPER_JOURNAL, 
		  PAPER_ABSTRACT, PAPER_NOTE, PAPER_PDF};
	enum {TAG_ID, TAG_NAME};

	Ui::PagePapersClass ui;

	QSqlTableModel modelPapers;
	QSqlTableModel modelAllTags;
	QSqlQueryModel modelTags;
	int currentRowPapers;
	int currentRowTags;
	QDataWidgetMapper* mapper;
};

#endif // PAGEPAPERS_H
