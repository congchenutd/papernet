#include "AddPhraseDlg.h"

AddPhraseDlg::AddPhraseDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

QString AddPhraseDlg::getPhrase() const {
	return ui.tePhrase->toPlainText();
}

QString AddPhraseDlg::getExplanation() const {
	return ui.teExplanation->toPlainText();
}

void AddPhraseDlg::setPhrase(const QString& phrase) {
	ui.tePhrase->setPlainText(phrase);
}

void AddPhraseDlg::setExplanation(const QString& explanation) {
	ui.teExplanation->setPlainText(explanation);
}
