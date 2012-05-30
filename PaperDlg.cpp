#include "PaperDlg.h"
#include "Common.h"
#include "Reference.h"
#include "EnglishName.h"
#include <QDate>
#include <QSqlTableModel>
#include <QCompleter>

PaperDlg::PaperDlg(QWidget *parent)
	: QDialog(parent), newPaper(true)
{
	ui.setupUi(this);
	resize(800, 600);

    // year
    int currentYear = QDate::currentDate().year();
    ui.sbYear->setValue(currentYear);
    ui.sbYear->setMaximum(currentYear);

    // auto complete for tags
	QSqlTableModel* tagModel = new QSqlTableModel(this);
	tagModel->setTable("Tags");
	tagModel->select();
	QCompleter* completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(tagModel);
	completer->setCompletionColumn(TAG_NAME);
	completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	ui.leTags->setCompleter(completer);
}

// getters
QString PaperDlg::getTitle() const {
	return ui.leTitle->text();
}

QStringList PaperDlg::getTags() const
{
    QStringList result;
    QString tagString = ui.leTags->text();
    if(tagString.isEmpty())
        return result;

    QStringList tags = tagString.split(";");
    foreach(const QString& tag, tags)
        result << tag.simplified();
    return result;
}

// setters
void PaperDlg::setTitle(const QString& title)
{
	ui.leTitle->setText(title);
	newPaper = title.isEmpty();
}

void PaperDlg::setType(const QString &type)
{
    int index = ui.comboType->findText(type);
    if(index > -1)
        ui.comboType->setCurrentIndex(index);
}

void PaperDlg::accept() {
	if(newPaper && paperExists(getTitle()))
		setWindowTitle(tr("Error: the title already exists!"));
	else
		QDialog::accept();
}

Reference PaperDlg::getReference() const
{
    Reference ref;
	ref.setValue("title",       getTitle());
    ref.setValue("publication", ui.lePublication->text().simplified());
    ref.setValue("publisher",   ui.lePublisher  ->text().simplified());
    ref.setValue("address",     ui.leAddress    ->text().simplified());
    ref.setValue("url",         ui.leUrl        ->text().simplified());

    ref.setValue("abstract", ui.teAbstract->toPlainText().simplified());
    ref.setValue("note",     ui.teNote    ->toPlainText().simplified());

    ref.setValue("editors", ui.leEditors->text().simplified());
	ref.setValue("authors", Reference::fromLineToList(ui.leAuthors->text()));
    ref.setValue("tags",    getTags());

    ref.setValue("year",      ui.sbYear     ->value());
    ref.setValue("volume",    ui.sbVolume   ->value());
    ref.setValue("issue",     ui.sbIssue    ->value());
    ref.setValue("startpage", ui.sbStartPage->value());
    ref.setValue("endpage",   ui.sbEndPage  ->value());

    ref.setValue("type", ui.comboType->currentText());
    return ref;
}

void PaperDlg::setReference(const Reference& ref)
{
	setType(ref.getValue("type").toString());
    ui.sbYear     ->setValue(ref.getValue("year")     .toInt());
    ui.sbVolume   ->setValue(ref.getValue("volume")   .toInt());
    ui.sbIssue    ->setValue(ref.getValue("issue")    .toInt());
    ui.sbStartPage->setValue(ref.getValue("startpage").toInt());
    ui.sbEndPage  ->setValue(ref.getValue("endpage")  .toInt());

    ui.leTitle      ->setText(ref.getValue("title")      .toString());
    ui.lePublication->setText(ref.getValue("publication").toString());
    ui.lePublisher  ->setText(ref.getValue("publisher")  .toString());
    ui.leAddress    ->setText(ref.getValue("address")    .toString());
    ui.leUrl        ->setText(ref.getValue("url")        .toString());

    ui.teAbstract->setPlainText(ref.getValue("abstract").toString());
    ui.teNote    ->setPlainText(ref.getValue("note")    .toString());

    ui.leAuthors->setText(ref.getValue("authors").toStringList().join("; "));
    ui.leEditors->setText(ref.getValue("editors").toStringList().join("; "));
    ui.leTags   ->setText(ref.getValue("tags")   .toStringList().join("; "));
}
