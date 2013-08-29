#include "PaperDlg.h"
#include "Common.h"
#include "Reference.h"
#include "../BibFixer/Convertor.h"
#include "RefFormatSpec.h"
#include "MultiSectionCompleter.h"
#include "OptionDlg.h"
#include <QSqlTableModel>
#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>

PaperDlg::PaperDlg(QWidget *parent)
    : QDialog(parent), _id(-1)
{
	ui.setupUi(this);
    resize(700, 600);
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
            << Field("note",        ui.teNote)
            << Field("PDF",         ui.lePDF);

    // auto complete for tags
	QSqlTableModel* tagModel = new QSqlTableModel(this);
	tagModel->setTable("Tags");
	tagModel->select();

    MultiSectionCompleter* completer = new MultiSectionCompleter(this);
    completer->setModel(tagModel, TAG_NAME);
    completer->setEdit(ui.leTags);
    completer->setSeparator("; ");

    // type combobox
    RefSpec* spec = RefSpecFactory::getInstance()->getSpec("bib");
    if(spec != 0)
        foreach(const TypeSpec& type, spec->getAllTypes())
            ui.comboType->addItem(type.getInternalName());
    if(ui.comboType->findText("unknown") == -1)
        ui.comboType->insertItem(0, "unknown");

    connect(ui.comboType, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(onTypeChanged(QString)));
    connect(ui.btGoogle,      SIGNAL(clicked()), this, SLOT(onGoogle()));
    connect(ui.btSelectPaper, SIGNAL(clicked()), this, SLOT(onSelectPaper()));
    connect(ui.btAddPDF,      SIGNAL(clicked()), this, SLOT(onAddPDF()));
}

void PaperDlg::setTitle(const QString& title)
{
    // remove protection and fix case
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
    else    // guess type from publication
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
    ref.setValue("type", ui.comboType->currentText());

    return ref;
}

void PaperDlg::setReference(const Reference& ref)
{
    _id = ref.getValue("id").toInt();

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

QString PaperDlg::getPDFPath() const {
    return ui.lePDF->text();
}

void PaperDlg::setPDFPath(const QString& path) {
    ui.lePDF->setText(path);
}

void PaperDlg::showMergeMark() {
    setWindowTitle(windowTitle() + " - Merged!");
}

void PaperDlg::onTypeChanged(const QString& typeName)
{
    // get required fields info from bibtex spec
    RefSpec* spec = RefSpecFactory::getInstance()->getSpec("bib");
    if(spec == 0)
        return;

    TypeSpec type = spec->getType(typeName);
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

void PaperDlg::onSelectPaper()
{
    emit selectPaper(_id);   // ask paper page to select the paper
    reject();
}

void PaperDlg::onAddPDF()
{
    QString lastPath = UserSetting::getInstance()->getLastAttachmentPath();
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open PDF File"),
                                                    lastPath,
                                                    tr("PDF (*.pdf)"));
    if(!filePath.isEmpty() && QFile::exists(filePath))
    {
        setPDFPath(filePath);
        UserSetting::getInstance()->setLastAttachmentPath(QFileInfo(filePath).path());
    }
}
