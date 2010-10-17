#include "PaperDlg.h"

PaperDlg::PaperDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

QString PaperDlg::getTitle() const {
	return ui.leTitle->text();
}

QString PaperDlg::getAuthors() const {
	return ui.leAuthors->text();
}

QString PaperDlg::getJournal() const {
	return ui.leJournal->text();
}

QString PaperDlg::getAbstract() const {
	return ui.teAbstract->toPlainText();
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

