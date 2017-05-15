#include "LinkDlg.h"

LinkDlg::LinkDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

QString LinkDlg::getName() const {
	return ui.leName->text();
}

QString LinkDlg::getUrl() const {
	return ui.leUrl->text();
}

