#ifndef PAPERTAGPAGE_H
#define PAPERTAGPAGE_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include "ui_PaperTagPage.h"
#include "Mode.h"

class Mode;
class PaperMode;
class TagMode;
class QDataWidgetMapper;

class PaperTagPage : public QWidget
{
	Q_OBJECT

public:
	PaperTagPage(QWidget *parent = 0);
	~PaperTagPage();

public slots:
	void onByWhat();
	void onCurrentRowPaperChanged(const QModelIndex& idx);
	void onAddPaper();
	void onDelPaper();
	void onSubmitPaper();
	void onAddTag();
	void onEditTag();
	void onDelTag();
	void onImport();
	void onSetPDF();
	void onReadPDF();
	void onSearch(const QString& target);
	void onCancelSearch();
	void onCurrentRowTagChanged();

private:
	void changeMode();
	void resetViews();
	QString makePDFFileName(const QString& title);
	QString getPDFPath() const;
	void updateRelatedPapers();
	void updateRelatedTags();
	void import(const QString& fileName, const QString& firstHead, 
				const QString& titleHead, const QString& authorHead, 
				const QString& journalHead, const QString& delimiter, 
				const QString& abstractHead = "NO_SUCH_HEAD");
	QString trimHead(const QString& line, const QString& delimiter) const;
	int getCurrentPaperID() const;
	int getCurrentTagID() const;
	int getPaperID(int row) const;
	int getTagID  (int row) const;
	QList<int> getSelectedTagIDs() const;

	friend class Mode;
	friend class PaperMode;
	friend class TagMode;

private:
	Ui::PaperTagPageClass ui;
	enum {PAPER_ID, PAPER_TITLE, PAPER_AUTHORS, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_PDF};
	enum {TAG_ID, TAG_NAME};
	typedef enum {BY_TAG, BY_PAPER} ByWhat;

	QSqlTableModel modelPapers;
	QSqlTableModel modelTags;
	QSqlQueryModel modelRelatedPapers;
	QSqlQueryModel modelRelatedTags;
	Mode* mode;
	int currentRowPaper;
	int currentRowTag;
	QDataWidgetMapper* mapper;
	ByWhat byWhat;
};

#endif // PAPERTAGPAGE_H
