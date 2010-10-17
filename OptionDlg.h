#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include <QDialog>
#include "ui_OptionDlg.h"
#include "../MySetting/MySetting.h"

class UserSetting;

class OptionDlg : public QDialog
{
	Q_OBJECT

public:
	OptionDlg(QWidget *parent = 0);
	void accept();

private slots:
	void onFont();

private:
	Ui::OptionDlgClass ui;

	UserSetting* setting;
};


class UserSetting : public MySetting<UserSetting>
{
public:
	UserSetting(const QString& fileName);

	QFont getFont() const;
	int   getBackupDays() const;
	bool  getKeepAttachments() const;
	QString getLastImportPath() const;
	QString getLastAttachmentPath() const;
	void setFont(const QFont& font);
	void setBackupDays(int days);
	void setKeepAttachments(bool keep);
	void setLastImportPath(const QString& path);
	void setLastAttachmentPath(const QString& path);

private:
	void loadDefaults();
};


#endif // OPTIONDLG_H
