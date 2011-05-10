#ifndef ADDTAGDLG_H
#define ADDTAGDLG_H

#include "ui_AddTagDlg.h"
#include <QSqlTableModel>

class AddTagDlg : public QDialog
{
	Q_OBJECT

public:
	AddTagDlg(QWidget* parent = 0);
	QString getText() const;
	void    setText(const QString& text);

private:
	Ui::AddTagDlg ui;
	QSqlTableModel model;
};

#endif // ADDTAGDLG_H
