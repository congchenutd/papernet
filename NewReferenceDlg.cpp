#include "NewReferenceDlg.h"
#include "ui_NewReferenceDlg.h"

NewReferenceDlg::NewReferenceDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewReferenceDlg)
{
    ui->setupUi(this);
}

NewReferenceDlg::~NewReferenceDlg() {
	delete ui;
}

QString NewReferenceDlg::getTitle() const {
	return ui->lineEdit->text().simplified();
}
