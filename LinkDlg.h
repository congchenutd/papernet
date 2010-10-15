#ifndef LINKDLG_H
#define LINKDLG_H

#include <QDialog>
#include "ui_LinkDlg.h"

class LinkDlg : public QDialog
{
	Q_OBJECT

public:
	LinkDlg(QWidget *parent = 0);
	~LinkDlg();

private:
	Ui::LinkDlgClass ui;
};

#endif // LINKDLG_H
