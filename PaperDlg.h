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

    Reference getReference() const;
    void      setReference(const Reference& ref);

private:
    void setTitle(const QString& title);
    void setType (const QString& type);

private:
	Ui::PaperDlgClass ui;
};

#endif // PAPERDLG_H
