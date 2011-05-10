#ifndef ADDSNIPPETDLG_H
#define ADDSNIPPETDLG_H

#include <QDialog>
#include <QStringListModel>
#include "ui_AddSnippetDlg.h"

// add/edit snippet, invoked from snippet page and paper page
class AddSnippetDlg : public QDialog
{
	Q_OBJECT

public:
	AddSnippetDlg(QWidget *parent = 0);

	void setSnippetID(int id);
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
	void onSwitchToSnippets();

private:
	Ui::AddSnippetDlgClass ui;
	QStringListModel model;
	int snippetID;
};

#endif // ADDSNIPPETDLG_H
