#include "AddTagDlg.h"
#include "Common.h"
#include <QCompleter>

AddTagDlg::AddTagDlg(QWidget* parent) :
	QDialog(parent)
{
	ui.setupUi(this);

	model.setTable("Tags");
	model.select();
	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(&model);
	completer->setCompletionColumn(TAG_NAME);
	ui.lineEdit->setCompleter(completer);
}

QString AddTagDlg::getText() const {
	return ui.lineEdit->text();
}

void AddTagDlg::setText(const QString& text) {
	ui.lineEdit->setText(text);
}
