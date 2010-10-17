#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class PaperDlg : public QDialog
{
	Q_OBJECT

public:
	PaperDlg(QWidget *parent = 0);

	QString getTitle   () const;
	QString getAuthors () const;
	QString getJournal () const;
	QString getAbstract() const;
	QString getNote    () const;
	void setTitle   (const QString& title);
	void setAuthors (const QString& authors);
	void setJournal (const QString& journal);
	void setAbstract(const QString& ab);
	void setNote    (const QString& note);

private:
	Ui::PaperDlgClass ui;
};

#endif // PAPERDLG_H
