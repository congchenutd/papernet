#ifndef ADDSNIPPETDLG_H
#define ADDSNIPPETDLG_H

#include <QDialog>
#include <QStringListModel>
#include "ui_AddSnippetDlg.h"

class AddSnippetDlg : public QDialog
{
	Q_OBJECT

public:
	AddSnippetDlg(QWidget *parent = 0);

	void setSnippetID(int id);
	void addPaper(const QString& title);

protected:
	virtual void resizeEvent(QResizeEvent*);

private slots:
	void onCurrentRowChanged();
	void onAdd();
	void onDel();
	void onSelect();
	virtual void accept();

private:

private:
	Ui::AddSnippetDlgClass ui;
	QStringListModel model;
	int snippetID;
};

#endif // ADDSNIPPETDLG_H
