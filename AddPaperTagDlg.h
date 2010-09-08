#ifndef ADDPAPERTAGDLG_H
#define ADDPAPERTAGDLG_H

#include <QDialog>
#include <QSqlTableModel>
#include <QList>
#include "ui_AddPaperTagDlg.h"

class AddPaperTagDlg : public QDialog
{
	Q_OBJECT

public:
	AddPaperTagDlg(QWidget *parent, const QString& tableName);
	QList<int> getSelected() const;

private:
	Ui::AddPaperTagDlgClass ui;
	QSqlTableModel model;
};

#endif // ADDPAPERTAGDLG_H
