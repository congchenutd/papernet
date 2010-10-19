#include "PaperDlg.h"
#include <QDate>

PaperDlg::PaperDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.sbYear->setValue(QDate::currentDate().year());
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