#include "PhraseDlg.h"
#include "Common.h"
#include <QCompleter>
#include <QSqlTableModel>

PhraseDlg::PhraseDlg(QWidget* parent)
    : QDialog(parent), _newPhrase(true)
{
	ui.setupUi(this);

    // auto completion for tags
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

QString PhraseDlg::getPhrase() const {
	return ui.tePhrase->toPlainText();
}
QString PhraseDlg::getExplanation() const {
	return ui.teExplanation->toPlainText();
}
void PhraseDlg::setPhrase(const QString& phrase)
{
	ui.tePhrase->setPlainText(phrase);
    _newPhrase = phrase.isEmpty();
}
void PhraseDlg::setExplanation(const QString& explanation) {
	ui.teExplanation->setPlainText(explanation);
}
QStringList PhraseDlg::getTags() const {
	return splitLine(ui.leTags->text(), ";");
}
void PhraseDlg::setTags(const QStringList &tags) {
	ui.leTags->setText(tags.join("; "));
}

void PhraseDlg::accept()
{
    // TODO: merge to existing phrase
    if(_newPhrase && phraseExists(getPhrase()))
		setWindowTitle(tr("Error: the phrase already exists!"));
	else
		QDialog::accept();
}
