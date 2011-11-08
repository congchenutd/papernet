#include "AddTagDlg.h"
#include "Common.h"
#include <QCompleter>

AddTagDlg::AddTagDlg(const QString& tableName, QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);

	model.setTable(tableName);
	model.select();

	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(&model);
	completer->setCompletionColumn(TAG_NAME);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	ui.lineEdit->setCompleter(completer);
}

QString AddTagDlg::getText() const {
	return ui.lineEdit->text().toLower();
}

void AddTagDlg::setText(const QString& text)
{
	ui.lineEdit->setText(text);
	ui.lineEdit->selectAll();
}

void AddTagDlg::accept() {
	if(!getText().isEmpty())
		QDialog::accept();
}
