#include "TagDlg.h"
#include "Common.h"
#include <QSqlTableModel>
#include <QCompleter>

TagDlg::TagDlg(const QString& tableName, QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);

    // auto completion
    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable(tableName);
    model->select();

	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(model);
	completer->setCompletionColumn(TAG_NAME);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	ui.lineEdit->setCompleter(completer);
}

QString TagDlg::getText() const {
	return ui.lineEdit->text().toLower();
}

void TagDlg::setText(const QString& text)
{
	ui.lineEdit->setText(text);
	ui.lineEdit->selectAll();
}

void TagDlg::accept() {
	if(!getText().isEmpty())
		QDialog::accept();
}
