#ifndef ADDTAGDLG_H
#define ADDTAGDLG_H

#include "ui_AddTagDlg.h"

class AddTagDlg : public QDialog
{
	Q_OBJECT

public:
	AddTagDlg(QAbstractItemModel* model, int column, QWidget* parent = 0);
	QString getText() const;

private:
	Ui::AddTagDlg ui;
};

#endif // ADDTAGDLG_H
