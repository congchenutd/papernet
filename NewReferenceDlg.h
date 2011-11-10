#ifndef NEWREFERENCEDLG_H
#define NEWREFERENCEDLG_H

#include <QDialog>

namespace Ui {
    class NewReferenceDlg;
}

class NewReferenceDlg : public QDialog
{
    Q_OBJECT

public:
	NewReferenceDlg(QWidget *parent = 0);
    ~NewReferenceDlg();

	QString getTitle() const;

private:
    Ui::NewReferenceDlg *ui;
};

#endif // NEWREFERENCEDLG_H
