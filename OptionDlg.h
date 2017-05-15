#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include <QDialog>
#include "ui_OptionDlg.h"
#include "MySetting.h"

class UserSetting;

class OptionDlg : public QDialog
{
	Q_OBJECT

public:
	OptionDlg(QWidget *parent = 0);
	void accept();

private slots:
	void onFont();
	void onRebuildFulltext();
	void onClearCache();
    void onSetBibFixerPath();

private:
	Ui::OptionDlgClass ui;
    UserSetting* _setting;
};


class UserSetting : public MySetting<UserSetting>
{
public:
	UserSetting(const QString& fileName);

	QFont   getFont()               const;
	int     getBackupDays()         const;
	bool    getKeepAttachments()    const;
    bool    getMoveAttachments()    const;
    bool    getExportToBibFixer()   const;
    QString getLastImportPath()     const;
	QString getLastAttachmentPath() const;
    QString getBibFixerPath()       const;
	QString getCompileDate()        const;
    QByteArray getSplitterSizes(const QString& splitterName) const;
	int     getPapersTabIndex() const;

	void setFont              (const QFont& font);
	void setBackupDays        (int days);
	void setKeepAttachments   (bool keep);
    void setMoveAttachments   (bool keep);
    void setExportToBibFixer  (bool exportToBibFixer);
	void setLastImportPath    (const QString& path);
	void setLastAttachmentPath(const QString& path);
    void setBibFixerPath  (const QString& path);
	void setSplitterSizes(const QString& splitterName, const QByteArray& sizes);
	void setPapersTabIndex(int tab);

private:
	void loadDefaults();
};


#endif // OPTIONDLG_H
