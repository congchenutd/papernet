#ifndef ADDPHRASEDLG_H
#define ADDPHRASEDLG_H

#include <QDialog>
#include "ui_AddPhraseDlg.h"

class AddPhraseDlg : public QDialog
{
	Q_OBJECT

public:
	AddPhraseDlg(QWidget *parent = 0);
	QString getPhrase()      const;
	QString getExplanation() const;
	void setPhrase     (const QString& phrase);
	void setExplanation(const QString& explanation);

private:
	Ui::AddPhraseDlg ui;
};

#endif // ADDPHRASEDLG_H
