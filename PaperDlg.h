#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class QCompleter;

class PaperDlg : public QDialog
{
	Q_OBJECT

public:
	PaperDlg(QWidget *parent = 0);

	QString getTitle   () const;
	QString getAuthors () const;
	int     getYear    () const;
	QString getJournal () const;
	QString getAbstract() const;
	QString getNote    () const;
	QStringList getTags() const;
	void setTitle   (const QString& title);
	void setAuthors (const QString& authors);
	void setYear    (int year);
	void setJournal (const QString& journal);
	void setAbstract(const QString& ab);
	void setNote    (const QString& note);
	void setTags    (const QStringList& tags);

private:
	Ui::PaperDlgClass ui;
	QCompleter* completer;
};

#endif // PAPERDLG_H
