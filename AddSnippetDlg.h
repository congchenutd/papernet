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

	void setPaperID  (int paper);
	void setSnippetID(int id)    { snippetID = id;    }
	QString getContent() const;
	QStringList getReferences() const;

private slots:
	void onCurrentRowChanged();
	void onAdd();
	void onDel();
	virtual void accept();

private:
	void addPaper(const QString& title);

private:
	Ui::AddSnippetDlgClass ui;
	QStringListModel model;
	int paperID;
	int snippetID;
};

#endif // ADDSNIPPETDLG_H
