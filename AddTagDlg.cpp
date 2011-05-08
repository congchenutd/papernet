#include "AddTagDlg.h"

AddTagDlg::AddTagDlg(QAbstractItemModel* model, int column, QWidget* parent) :
	QDialog(parent)
{
	ui.setupUi(this);
//	ui.comboBox->setModel(model);
//	ui.comboBox->setModelColumn(column);
	ui.comboBox->loadFromTable("Tags", 1);
	ui.comboBox->setEditable(true);
}

QString AddTagDlg::getText() const {
	return ui.comboBox->currentText();
}

void AddTagDlg::setText(const QString &text)
{
//	ui.comboBox->setCu
}
