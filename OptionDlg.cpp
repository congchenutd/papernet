#include "OptionDlg.h"
#include <QFontDialog>

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
	ui.checkKeepAttachments->setChecked(setting->getKeepAttachments());
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
	setting->setKeepAttachments(ui.checkKeepAttachments->isChecked());

	QDialog::accept();
}

//////////////////////////////////////////////////////////////////////////
// Setting
UserSetting::UserSetting(const QString& fileName) : MySetting<UserSetting>(fileName)
{
	if(QFile(this->fileName).size() == 0)   // no setting
		loadDefaults();
}

void UserSetting::loadDefaults()
{
	setFont(qApp->font());
	setBackupDays(7);
	setKeepAttachments(false);
	setLastImportPath(".");
	setLastAttachmentPath(".");
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
bool UserSetting::getKeepAttachments() const {
	return value("KeepAttachments").toBool();
}
QString UserSetting::getLastImportPath() const {
	return value("LastImportPath").toString();
}
QString UserSetting::getLastAttachmentPath() const {
	return value("LastAttachmentPath").toString();
}

void UserSetting::setFont(const QFont& font) {
	setValue("Font", font);
}
void UserSetting::setBackupDays(int days) {
	setValue("BackupDays", days);
}
void UserSetting::setKeepAttachments(bool keep) {
	setValue("KeepPDF", keep);
}
void UserSetting::setLastImportPath(const QString& path) {
	setValue("LastImportPath", path);
}
void UserSetting::setLastAttachmentPath(const QString& path) {
	setValue("LastAttrachmentPath", path);
}