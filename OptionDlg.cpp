#include "OptionDlg.h"
#include <QFontDialog>
#include <QFileDialog>
#include <cstdlib>
#include <QProcessEnvironment>
#include <QResource>

OptionDlg::OptionDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setting = MySetting<UserSetting>::getInstance();

	connect(ui.btFont, SIGNAL(clicked()), this, SLOT(onFont()));
	connect(ui.btTemp, SIGNAL(clicked()), this, SLOT(onTempLocation()));

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
	ui.leTemp->setText(setting->getTempLocation());
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
	setting->setTempLocation(ui.leTemp->text());

	QDialog::accept();
}

void OptionDlg::onTempLocation()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		setting->getTempLocation(),	QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(!dir.isEmpty())
		ui.leTemp->setText(dir);
}

//////////////////////////////////////////////////////////////////////////
// Setting
UserSetting::UserSetting(const QString& fileName) 
: MySetting<UserSetting>(fileName)
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
	setValue("SmallIcon", false);
	setValue("ShowText",  false);
	setTempLocation(QProcessEnvironment::systemEnvironment().value("TMP", "."));
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
QString UserSetting::getTempLocation() const {
	return value("TempLocation").toString();
}

void UserSetting::setFont(const QFont& font) {
	setValue("Font", font);
}
void UserSetting::setBackupDays(int days) {
	setValue("BackupDays", days);
}
void UserSetting::setKeepAttachments(bool keep) {
    setValue("KeepAttachments", keep);
}
void UserSetting::setLastImportPath(const QString& path) {
	setValue("LastImportPath", path);
}
void UserSetting::setLastAttachmentPath(const QString& path) {
    setValue("LastAttachmentPath", path);
}
void UserSetting::setTempLocation(const QString& temp) {
	setValue("TempLocation", temp);
}

QString UserSetting::getCompileDate() const
{
	// this resource file will be generated after running CompileDate.bat
	QResource resource(":/MainWindow/CompileDate.txt");
	QString result = (char*)resource.data();
	return result.isEmpty() ? "Unknown" : result;
}
