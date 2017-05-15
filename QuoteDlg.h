#ifndef QUOTEDLG_H
#define QUOTEDLG_H

#include <QDialog>
#include <QStringListModel>
#include "ui_QuoteDlg.h"

// add/edit quote, invoked by the quote page and paper page
class QuoteDlg : public QDialog
{
	Q_OBJECT

public:
    QuoteDlg(QWidget *parent = 0);

    void setQuoteID(int id);              // set the id of the quote this dlg works on
    void addRef(const QString& title);    // add a paper to this quote
	virtual void accept();

private slots:
	void onCurrentRowChanged();
	void onAddRef();               // create a new ref
	void onDelRef();
	void onSelectRef();            // select existing ref
    void onGotoPapersPage();
    void onGotoQuotesPage();
    void onViewPDF();

private:
    Ui::QuoteDlg ui;
    QStringListModel _model;        // list the references
    int _quoteID;
    int _selectedPaperID;
};

#endif // QUOTEDLG_H
