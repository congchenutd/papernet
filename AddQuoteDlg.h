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

protected:
	virtual void resizeEvent(QResizeEvent*);

private slots:
	void onCurrentRowChanged();
	void onAddRef();
	void onDelRef();
	void onSelectRef();            // select existing ref
	virtual void accept();
	void onSwitchToPapers();
	void onSwitchToQuotes();

private:
	Ui::AddQuoteDlgClass ui;
	QStringListModel model;
	int quoteID;
};

#endif // ADDQUOTEDLG_H
