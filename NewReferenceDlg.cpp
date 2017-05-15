#include "NewReferenceDlg.h"
#include "Common.h"

NewReferenceDlg::NewReferenceDlg(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    resize(800, 30);
}

QString NewReferenceDlg::getTitle() const {
    return ui.lineEdit->text().simplified();
}
