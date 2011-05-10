#include "AddTagDlg.h"
#include "Common.h"
#include <QCompleter>

AddTagDlg::AddTagDlg(QWidget* parent) :
	QDialog(parent)
{
	ui.setupUi(this);

	// a compeleter gives a hint of the input
	model.setTable("Tags");
	model.select();
	model.sort(TAG_NAME, Qt::AscendingOrder);
	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(&model);
	completer->setCompletionColumn(TAG_NAME);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	ui.lineEdit->setCompleter(completer);
}

QString AddTagDlg::getText() const {
	return ui.lineEdit->text();
}

void AddTagDlg::setText(const QString& text)
{
	ui.lineEdit->setText(text);
	ui.lineEdit->selectAll();
}
