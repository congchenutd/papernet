#include "PaperDlg.h"
#include "Common.h"
#include "Reference.h"
#include "EnglishName.h"
#include "../BibFixer/Convertor.h"
#include <QDate>
#include <QSqlTableModel>
#include <QCompleter>

PaperDlg::PaperDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	resize(800, 700);
	ui.leTitle->setFocus();

    // default year
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

void PaperDlg::setTitle(const QString& title)
{
    QString fixedTitle = BibFixer::CaseConvertor().convert(
                BibFixer::UnprotectionConvertor().convert(title));
    ui.leTitle->setText(fixedTitle);
	ui.leTitle->setCursorPosition(0);
}

void PaperDlg::setType(const QString& type)
{
    int index = ui.comboType->findText(type);
    if(index > -1)
        ui.comboType->setCurrentIndex(index);
    else    // guess type
    {
        QString publication = ui.lePublication->text();
        if(publication.contains("proceeding", Qt::CaseInsensitive) ||
           publication.contains("proc.",      Qt::CaseInsensitive))
            ui.comboType->setCurrentText("inproceedings");

        else if(publication.contains("journal",     Qt::CaseInsensitive) ||
                publication.contains("j.",          Qt::CaseInsensitive) ||
                publication.contains("transaction", Qt::CaseInsensitive) ||
                publication.contains("trans.",      Qt::CaseInsensitive))
            ui.comboType->setCurrentText("journal");
    }
}

void PaperDlg::accept()
{
    if(ui.leTitle->text().isEmpty())
        ui.leTitle->setFocus();   // force non-empty title
    else
		QDialog::accept();
}

Reference PaperDlg::getReference() const
{
    Reference ref;
    ref.setValue("title",       ui.leTitle      ->text().simplified());
    ref.setValue("publication", ui.lePublication->text().simplified());
    ref.setValue("publisher",   ui.lePublisher  ->text().simplified());
    ref.setValue("address",     ui.leAddress    ->text().simplified());
    ref.setValue("url",         ui.leUrl        ->text().simplified());
    ref.setValue("editors",     ui.leEditors    ->text().simplified());
    ref.setValue("startpage",   ui.leStartPage  ->text().simplified());
    ref.setValue("endpage",     ui.leEndPage    ->text().simplified());

    ref.setValue("year",      ui.sbYear  ->value());
    ref.setValue("volume",    ui.sbVolume->value());
    ref.setValue("issue",     ui.sbIssue ->value());

    ref.setValue("type", ui.comboType->currentText());
    ref.setValue("abstract", ui.teAbstract->toPlainText().trimmed());
    ref.setValue("note",     ui.teNote    ->toPlainText().trimmed());

    // authors and tags are QStringLists
    ref.setValue("authors", splitAuthorsList(ui.leAuthors->text()));
    ref.setValue("tags",    splitLine(ui.leTags->text(), ";"));

    return ref;
}

void PaperDlg::setReference(const Reference& ref)
{
    setTitle(ref.getValue("title").toString());

    ui.lePublication->setText(ref.getValue("publication").toString());
    ui.lePublisher  ->setText(ref.getValue("publisher")  .toString());
    ui.leAddress    ->setText(ref.getValue("address")    .toString());
    ui.leUrl        ->setText(ref.getValue("url")        .toString());
    ui.leStartPage  ->setText(ref.getValue("startpage")  .toString());
    ui.leEndPage    ->setText(ref.getValue("endpage")    .toString());

    ui.sbYear  ->setValue(ref.getValue("year")  .toInt());
    ui.sbVolume->setValue(ref.getValue("volume").toInt());
    ui.sbIssue ->setValue(ref.getValue("issue") .toInt());

    ui.leAuthors->setText(ref.getValue("authors").toStringList().join("; "));
    ui.leEditors->setText(ref.getValue("editors").toStringList().join("; "));

    // merge tags
    QStringList tags = splitLine(ui.leTags->text(), ";");
    tags << ref.getValue("tags").toStringList();
    ui.leTags->setText(tags.join("; "));

    // do not overwrite abstract and note
    if(ui.teAbstract->toPlainText().isEmpty())
        ui.teAbstract->setPlainText(ref.getValue("abstract").toString());

    if(ui.teNote->toPlainText().isEmpty())
        ui.teNote->setPlainText(ref.getValue("note").toString());

    ui.leAuthors    ->setCursorPosition(0);
    ui.lePublication->setCursorPosition(0);

    // set type after publication, because we may guess type from publication
    setType(ref.getValue("type").toString());
}
