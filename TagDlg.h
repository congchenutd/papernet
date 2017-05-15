#ifndef TAGDLG_H
#define TAGDLG_H

#include "ui_TagDlg.h"

// add/edit a tag
class TagDlg : public QDialog
{
	Q_OBJECT

public:
    TagDlg(const QString& tableName, QWidget* parent = 0);
	QString getText() const;
	void    setText(const QString& text);
	virtual void accept();

private:
    Ui::TagDlg ui;
};

#endif // TAGDLG_H
