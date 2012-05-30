#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class Reference;

// add/edit paper
class PaperDlg : public QDialog
{
	Q_OBJECT

public:
	PaperDlg(QWidget *parent = 0);
	virtual void accept();

	QString     getTitle() const;
    QStringList getTags () const;
	void setTitle(const QString& title);
	void setType (const QString& type);

    Reference getReference() const;
    void      setReference(const Reference& ref);

private:
	Ui::PaperDlgClass ui;
	bool newPaper;
};

#endif // PAPERDLG_H
