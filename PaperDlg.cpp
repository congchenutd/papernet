#include "PaperDlg.h"
#include "Common.h"
#include <QDate>
#include <QSqlTableModel>
#include <QCompleter>

PaperDlg::PaperDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	resize(800, 600);
	ui.sbYear->setValue(QDate::currentDate().year());

	QSqlTableModel* tagModel = new QSqlTableModel(this);
	tagModel->setTable("Tags");
	tagModel->select();

	completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(tagModel);
	completer->setCompletionColumn(TAG_NAME);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);

	ui.leTags->setCompleter(completer);
}

QString PaperDlg::getTitle() const {
	return ui.leTitle->text().trimmed();
}

QString PaperDlg::getAuthors() const {
	return ui.leAuthors->text().trimmed();
}

int PaperDlg::getYear() const {
	return ui.sbYear->value();
}

QString PaperDlg::getJournal() const {
	return ui.leJournal->text().trimmed();
}

QString PaperDlg::getAbstract() const {
	return ui.teAbstract->toPlainText().trimmed();
}

QString PaperDlg::getNote() const {
	return ui.teNote->toPlainText();
}

QStringList PaperDlg::getTags() const {
	return ui.leTags->text().isEmpty() ? QStringList()
									   : ui.leTags->text().split(";");
}

void PaperDlg::setTitle(const QString& title) {
	ui.leTitle->setText(title);
}

void PaperDlg::setAuthors(const QString& authors) {
	ui.leAuthors->setText(authors);
}

void PaperDlg::setJournal(const QString& journal) {
	ui.leJournal->setText(journal);
}

void PaperDlg::setAbstract(const QString& ab) {
	ui.teAbstract->setPlainText(ab);
}

void PaperDlg::setNote(const QString& note) {
	ui.teNote->setPlainText(note);
}

void PaperDlg::setYear(int year){
	ui.sbYear->setValue(year);
}

void PaperDlg::setTags(const QStringList& tags) {
	ui.leTags->setText(tags.join(";"));
}