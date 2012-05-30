#include "AddPhraseDlg.h"
#include "Common.h"
#include <QCompleter>
#include <QSqlTableModel>

AddPhraseDlg::AddPhraseDlg(QWidget *parent)
	: QDialog(parent), newPhrase(true)
{
	ui.setupUi(this);

	QSqlTableModel* tagModel = new QSqlTableModel(this);
	tagModel->setTable("DictionaryTags");
	tagModel->select();
	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(tagModel);
	completer->setCompletionColumn(TAG_NAME);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	ui.leTags->setCompleter(completer);
}

QString AddPhraseDlg::getPhrase() const {
	return ui.tePhrase->toPlainText();
}
QString AddPhraseDlg::getExplanation() const {
	return ui.teExplanation->toPlainText();
}
void AddPhraseDlg::setPhrase(const QString& phrase)
{
	ui.tePhrase->setPlainText(phrase);
	newPhrase = phrase.isEmpty();
}
void AddPhraseDlg::setExplanation(const QString& explanation) {
	ui.teExplanation->setPlainText(explanation);
}
QStringList AddPhraseDlg::getTags() const {
	return splitLine(ui.leTags->text(), ";");
}
void AddPhraseDlg::setTags(const QStringList &tags) {
	ui.leTags->setText(tags.join("; "));
}

void AddPhraseDlg::accept()
{
	if(newPhrase && phraseExists(getPhrase()))
		setWindowTitle(tr("Error: the phrase already exists!"));
	else
		QDialog::accept();
}
