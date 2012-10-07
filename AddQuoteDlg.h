#ifndef ADDQUOTEDLG_H
#define ADDQUOTEDLG_H

#include <QDialog>
#include <QStringListModel>
#include "ui_AddQuoteDlg.h"

// add/edit quote, invoked from quote page and paper page
class AddQuoteDlg : public QDialog
{
	Q_OBJECT

public:
	AddQuoteDlg(QWidget *parent = 0);

	void setQuoteID(int id);
	void addRef(const QString& title);
	virtual void accept();

protected:
	virtual void resizeEvent(QResizeEvent*);

private slots:
	void onCurrentRowChanged();
	void onAddRef();               // create a new ref
	void onDelRef();
	void onSelectRef();            // select existing ref
	void onSwitchToPapers();
    void onGotoQuotePage();
    void onViewPDF();

private:
	Ui::AddQuoteDlgClass ui;
	QStringListModel model;        // list the references
	int quoteID;
    int selectedPaperID;
};

#endif // ADDQUOTEDLG_H
