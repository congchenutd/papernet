#include "AddTagDlg.h"
#include <QCompleter>

AddTagDlg::AddTagDlg(QAbstractItemModel* model, int column, QWidget* parent) :
	QDialog(parent)
{
	ui.setupUi(this);
	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(model);
	completer->setCompletionColumn(column);
	ui.lineEdit->setCompleter(completer);
}

QString AddTagDlg::getText() const {
	return ui.lineEdit->text();
}