#ifndef LINKDLG_H
#define LINKDLG_H

#include <QDialog>
#include "ui_LinkDlg.h"

// Add hyperlink to a paper
class LinkDlg : public QDialog
{
	Q_OBJECT

public:
	LinkDlg(QWidget *parent = 0);
	QString getName() const;
	QString getUrl()  const;

private:
	Ui::LinkDlgClass ui;
};

#endif // LINKDLG_H
