#ifndef ADDTAGDLG_H
#define ADDTAGDLG_H

#include "ui_AddTagDlg.h"
#include <QSqlTableModel>

// add/edit a tag
class AddTagDlg : public QDialog
{
	Q_OBJECT

public:
	AddTagDlg(const QString& tableName, QWidget* parent = 0);
	QString getText() const;
	void    setText(const QString& text);
	void    accept();

private:
	Ui::AddTagDlg ui;
	QSqlTableModel model;
};

#endif // ADDTAGDLG_H
