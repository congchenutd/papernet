#ifndef PAGEPAPERS_H
#define PAGEPAPERS_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include "ui_PagePapers.h"
#include "PaperModel.h"

struct ImportResult;
class PaperDlg;

class PagePapers : public QWidget
{
	Q_OBJECT

public:
	PagePapers(QWidget* parent = 0);
	~PagePapers();
	void jumpToPaper(const QString& title);
	void saveGeometry();

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

	void onAddQuote();
	void onEditQuote(const QModelIndex& idx);
	void onDelQuotes();

signals:
	void tableValid(bool);

private:
	int  getPaperID(int row) const;
	int  idToRow   (int id)  const;
	void selectID(int id);
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
