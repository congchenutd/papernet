#include "PaperDlg.h"
#include "Common.h"
#include "Reference.h"
#include "EnglishName.h"
#include <QDate>
#include <QSqlTableModel>
#include <QCompleter>

PaperDlg::PaperDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	resize(800, 700);
	ui.leTitle->setFocus();

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

// setters
void PaperDlg::setTitle(const QString& title)
{
	ui.leTitle->setText(title);
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
    if(getTitle().isEmpty())
        ui.leTitle->setFocus();
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

    ref.setValue("abstract", ui.teAbstract->toPlainText().trimmed());
    ref.setValue("note",     ui.teNote    ->toPlainText().trimmed());

    ref.setValue("editors", ui.leEditors->text().simplified());
	ref.setValue("authors", splitAuthorsList(ui.leAuthors->text()));
	ref.setValue("tags",    splitLine(ui.leTags->text(), ";"));

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
    ui.sbYear     ->setValue(ref.getValue("year")     .toInt());
    ui.sbVolume   ->setValue(ref.getValue("volume")   .toInt());
    ui.sbIssue    ->setValue(ref.getValue("issue")    .toInt());
    ui.sbStartPage->setValue(ref.getValue("startpage").toInt());
    ui.sbEndPage  ->setValue(ref.getValue("endpage")  .toInt());

	setTitle(ref.getValue("title").toString());
    ui.lePublication->setText(ref.getValue("publication").toString());
    ui.lePublisher  ->setText(ref.getValue("publisher")  .toString());
    ui.leAddress    ->setText(ref.getValue("address")    .toString());
    ui.leUrl        ->setText(ref.getValue("url")        .toString());

    ui.teAbstract->setPlainText(ref.getValue("abstract").toString());
    ui.teNote    ->setPlainText(ref.getValue("note")    .toString());

    ui.leAuthors->setText(ref.getValue("authors").toStringList().join("; "));
    ui.leEditors->setText(ref.getValue("editors").toStringList().join("; "));
    ui.leTags   ->setText(ref.getValue("tags")   .toStringList().join("; "));

    ui.leAuthors->setCursorPosition(0);
    ui.lePublication->setCursorPosition(0);

    // set type after publication, because we may guess type from publication
    setType(ref.getValue("type").toString());
}
