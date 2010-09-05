#include "OptionDlg.h"
#include <QtGui>

OptionDlg::OptionDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.btFont, SIGNAL(clicked()), this, SLOT(onFont()));

	setting = MySetting<UserSetting>::getInstance();

	// load settings
	qApp->setFont(setting->getFont());
	int backupDays = setting->getBackupDays();
	if(backupDays == 0)
	{
		ui.checkAutoBack->setChecked(false);
		ui.sbBackupDays->setEnabled(false);
	}
	ui.sbBackupDays->setValue(backupDays);
}

void OptionDlg::onFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, setting->getFont(), this);
	if(ok)
		setting->setFont(font);
}

void OptionDlg::accept()
{
	// apply settings
	qApp->setFont(setting->getFont());
	setting->setBackupDays(ui.checkAutoBack->isChecked() ? ui.sbBackupDays->value() : 0);

	QDialog::accept();
}

//////////////////////////////////////////////////////////////////////////
// Setting
UserSetting::UserSetting(const QString& fileName) : MySetting<UserSetting>(fileName)
{
	if(QFile(fileName).size() == 0)   // no setting
		loadDefaults();
}

void UserSetting::loadDefaults()
{
	setFont(qApp->font());
	setBackupDays(7);
}

QFont UserSetting::getFont() const
{
	QFont font;
	font.fromString(value("Font").toString());
	return font;
}

int UserSetting::getBackupDays() const {
	return value("BackupDays").toInt();
}

void UserSetting::setFont(const QFont& font) {
	setValue("Font", font);
}

void UserSetting::setBackupDays(int days) {
	setValue("BackupDays", days);
}
