#ifndef NEWREFERENCEDLG_H
#define NEWREFERENCEDLG_H

#include <QDialog>
#include "ui_NewReferenceDlg.h"

// for adding a simple new reference used by QuoteDlg
class NewReferenceDlg : public QDialog
{
    Q_OBJECT

public:
	NewReferenceDlg(QWidget *parent = 0);
	QString getTitle() const;

private:
    Ui::NewReferenceDlg ui;
};

#endif // NEWREFERENCEDLG_H
