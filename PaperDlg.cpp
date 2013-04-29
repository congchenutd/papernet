#include "PaperDlg.h"
#include "Common.h"
#include "Reference.h"
#include "../BibFixer/Convertor.h"
#include "RefFormatSpec.h"
#include <QSqlTableModel>
#include <QCompleter>
#include <QUrl>
#include <QDesktopServices>

PaperDlg::PaperDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	resize(800, 700);
	ui.leTitle->setFocus();

    // fields, type and tags not included
    _fields << Field("title",       ui.leTitle)
            << Field("authors",     ui.leAuthors)
            << Field("publication", ui.lePublication)
            << Field("year",        ui.leYear)
            << Field("volume",      ui.leVolume)
            << Field("issue",       ui.leIssue)
            << Field("startpage",   ui.leStartPage)
            << Field("endpage",     ui.leEndPage)
            << Field("editors",     ui.leEditors)
            << Field("address",     ui.leAddress)
            << Field("publisher",   ui.lePublisher)
            << Field("url",         ui.leUrl)
            << Field("abstract",    ui.teAbstract)
            << Field("note",        ui.teNote);

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

    _spec = RefSpecFactory::getInstance()->getSpec("bib");

    // type combobox
    if(_spec != 0)
        foreach(const TypeSpec& type, _spec->getAllTypes())
            ui.comboType->addItem(type.getInternalName());
    if(ui.comboType->findText("unknown") == -1)
        ui.comboType->insertItem(0, "unknown");

    connect(ui.comboType, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTypeChanged(QString)));
    connect(ui.btGoogle,  SIGNAL(clicked()), this, SLOT(onGoogle()));
}

void PaperDlg::setTitle(const QString& title)
{
    // remove protectin and fix case
    QString fixedTitle = BibFixer::CaseConvertor().redo(
                BibFixer::ProtectionConvertor().undo(title));
    ui.leTitle->setText(fixedTitle);
	ui.leTitle->setCursorPosition(0);
    ui.btGoogle->setEnabled(!title.isEmpty());
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

    foreach(const Field& field, _fields)
        if(field.first != "authors" && field.first != "editors" && field.first != "tags")
            ref.setValue(field.first, field.second->text().simplified());

    // abstract and note may contain returns
    ref.setValue("abstract", ui.teAbstract->toPlainText().trimmed());
    ref.setValue("note",     ui.teNote    ->toPlainText().trimmed());

    // authors, editors, and tags are QStringLists
    ref.setValue("editors", splitAuthorsList(ui.leEditors->text()));
    ref.setValue("authors", splitAuthorsList(ui.leAuthors->text()));
    ref.setValue("tags",    splitLine(ui.leTags->text(), ";"));

    // type is not included in _fields
    ref.setValue("type",     ui.comboType->currentText());

    return ref;
}

void PaperDlg::setReference(const Reference& ref)
{
    foreach(const Field& field, _fields)
        if(field.second->text().isEmpty())   // do not overwrite
            field.second->setText(ref.getValue(field.first).toString());

    setTitle(ref.getValue("title").toString());    // fix the tile

    // names are QStringLists
    ui.leAuthors->setText(ref.getValue("authors").toStringList().join("; "));
    ui.leEditors->setText(ref.getValue("editors").toStringList().join("; "));

    // merge tags
    QStringList tags = splitLine(ui.leTags->text(), ";");
    tags << ref.getValue("tags").toStringList();
    ui.leTags->setText(tags.join("; "));

    // move the cursor to the front, otherwise it shows the end of the line
    ui.leAuthors    ->setCursorPosition(0);
    ui.lePublication->setCursorPosition(0);

    // set type after setting publication, thus we may guess the type from publication
    setType(ref.getValue("type").toString());
}

void PaperDlg::showMergeMark() {
    setWindowTitle(windowTitle() + " - Merged!");
}

void PaperDlg::onTypeChanged(const QString& typeName)
{
    // get required fields info from bibtex spec
    if(_spec == 0)
        return;

    TypeSpec type = _spec->getType(typeName);
    for(Fields::iterator it = _fields.begin(); it != _fields.end(); ++ it)
        it->second->highlight(type.isRequiredField(it->first) // invalid type returns false
                              ? QColor(Qt::yellow).lighter()
                              : palette().base().color());    // reset palette
}

void PaperDlg::onGoogle() {
    if(!ui.leTitle->text().isEmpty())
        QDesktopServices::openUrl(
                    QUrl("http://www.google.com/search?q=" + ui.leTitle->text()));
}
