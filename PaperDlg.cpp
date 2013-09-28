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
    : QDialog(parent), _id(-1), _dirty(false)
{
	ui.setupUi(this);
    resize(750, 650);

    _fields << Field("title",       ui.leTitle)
            << Field("authors",     ui.leAuthors)
            << Field("publication", ui.lePublication)
            << Field("year",        ui.leYear)
            << Field("volume",      ui.leVolume)
            << Field("issue",       ui.leIssue)
            << Field("startpage",   ui.leStartPage)
            << Field("endpage",     ui.leEndPage)
			<< Field("type",        ui.comboType)
            << Field("editors",     ui.leEditors)
            << Field("address",     ui.leAddress)
            << Field("publisher",   ui.lePublisher)
            << Field("url",         ui.leUrl)
            << Field("abstract",    ui.teAbstract)
            << Field("note",        ui.teNote)
            << Field("PDF",         ui.lePDF)        // saving PDF path
            << Field("tags",        ui.leTags);

    ui.leAuthors->setSeparator("; ");
    ui.leEditors->setSeparator("; ");
    ui.leTags   ->setSeparator("; ");
    ui.lePDF->hide();  // a special field for pdf path

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
    ui.comboType->setCurrentText("unknown");

    connect(ui.comboType, SIGNAL(currentIndexChanged(QString)),
            this,         SLOT(onTypeChanged(QString)));
    connect(ui.btGoogle,  SIGNAL(clicked()), this, SLOT(onGoogle()));
    connect(ui.btAddPDF,  SIGNAL(clicked()), this, SLOT(onAddPDF()));

    // these signals won't be triggered by program
    connect(ui.leTitle,       SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leAuthors,     SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.lePublication, SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leYear,        SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leVolume,      SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leIssue,       SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leStartPage,   SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leEndPage,     SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leEditors,     SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leAddress,     SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.lePublisher,   SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leUrl,         SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.lePDF,         SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
    connect(ui.leTags,        SIGNAL(textEdited(QString)), this, SLOT(onDirty()));
}

void PaperDlg::setTitle(const QString& title)
{
    // remove protection and fix case
    QString fixedTitle = BibFixer::CaseConvertor().redo(
                BibFixer::ProtectionConvertor().undo(title));
    ui.leTitle->setText(fixedTitle);
    ui.btGoogle->setEnabled(!fixedTitle.isEmpty());
}

void PaperDlg::setType(const QString& type)
{
    int index = ui.comboType->findText(type);
    if(index > -1)
        ui.comboType->setCurrentIndex(index);
    else    // guess type from publication
    {
        QString publication = ui.lePublication->text().toLower();
        if(publication.contains("proceeding") ||
           publication.contains("proc.")      ||
		   publication.contains("conference") ||
		   publication.contains("conf."))
            ui.comboType->setCurrentText("inproceedings");

        else if(publication.contains("journal") ||
                publication.contains("j.") ||
                publication.contains("transaction") ||
                publication.contains("trans."))
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
		ref.setValue(field.first, field.second->text().simplified());
    return ref;
}

void PaperDlg::setReference(const Reference& ref)
{
    // avoid triggering dirty signals by the following code
    disableDirtyConnections();

    _id = ref.fieldExists("id") ? ref.getValue("id").toInt() : -1;

    foreach(const Field& field, _fields)
        if(field.second->text().isEmpty())   // do not overwrite
            field.second->setText(ref.getValue(field.first).toString());

    setTitle(ref.getValue("title").toString());    // fix the tile

    // call it after setting publication, so we may guess the type from publication
    setType(ref.getValue("type").toString());

    // connect after the fields are modified by program
    enableDirtyConnections();
}

void PaperDlg::enableDirtyConnections()
{
    connect(ui.comboType,  SIGNAL(currentTextChanged(QString)), this, SLOT(onDirty()));
    connect(ui.teAbstract, SIGNAL(textChanged()), this, SLOT(onDirty()));
    connect(ui.teNote,     SIGNAL(textChanged()), this, SLOT(onDirty()));
}

void PaperDlg::disableDirtyConnections()
{
    disconnect(ui.comboType,  SIGNAL(currentTextChanged(QString)), this, SLOT(onDirty()));
    disconnect(ui.teAbstract, SIGNAL(textChanged()), this, SLOT(onDirty()));
    disconnect(ui.teNote,     SIGNAL(textChanged()), this, SLOT(onDirty()));
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

void PaperDlg::onAddPDF()
{
    QString lastPath = UserSetting::getInstance()->getLastAttachmentPath();
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open PDF File"),
                                                    lastPath,
                                                    tr("PDF (*.pdf)"));
    if(!filePath.isEmpty() && QFile::exists(filePath))
    {
        ui.lePDF->setText(filePath);
        ui.btAddPDF->setText(ui.btAddPDF->text() + "*");   // indicated PDF is added
        UserSetting::getInstance()->setLastAttachmentPath(QFileInfo(filePath).path());
        onDirty();
    }
}

void PaperDlg::onDirty()
{
    _dirty = true;
    ui.btOK->setEnabled(true);
}
