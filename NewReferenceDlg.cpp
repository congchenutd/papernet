#include "NewReferenceDlg.h"

NewReferenceDlg::NewReferenceDlg(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
}

QString NewReferenceDlg::getTitle() const {
    return ui.lineEdit->text().simplified();
}
