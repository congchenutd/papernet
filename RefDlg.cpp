#include "RefDlg.h"
#include "RefFormatSpec.h"
#include "RefParser.h"
#include "SetTextComboBox.h"
#include <QLabel>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QLineEdit>

RefDlg::RefDlg(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    // type combobox
    _comboType = new SetTextComboBox(this);
    _spec = RefSpecFactory::getInstance()->getSpec("bib");
    if(_spec != 0)
        foreach(const TypeSpec& type, _spec->getAllTypes())
            _comboType->addItem(type.getInternalName());
    if(_comboType->findText("unknown") == -1)
        _comboType->insertItem(0, "unknown");

    connect(_comboType, SIGNAL(currentIndexChanged(QString)), this, SLOT(reloadFields(QString)));

    // layout
    _layout = new QGridLayout(this);
    ui.tabRef->setLayout(_layout);
    _layout->addWidget(new QLabel("Type"), 0, 0);
    _layout->addWidget(_comboType, 0, 1);   // the type field is always there

    _spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void RefDlg::setSource(const QString& bibtexSource)
{
    if(_spec == 0)
        return;

    // parse the source and add the result
    QList<Reference> refs = _spec->getParser()->parse(bibtexSource, _spec);
    if(!refs.isEmpty())
        setReference(refs.front());   // in case of multi refs, use the 1st one
}

void RefDlg::setReference(const Reference& reference)
{
    _currentRef = reference;               // backup for reloadFields()
    QString typeName = reference.getValue("type").toString();
    _comboType->setCurrentText(typeName);  // may trigger reloadFields()
    reloadFields(typeName);
}

void RefDlg::reloadFields(const QString& typeName)
{
    clearFields();

    // reset the fields based on the fields of the type spec
    if(_spec != 0)
        foreach(const FieldSpec& fieldSpec, _spec->getType(typeName).getAllFields())
            createField(fieldSpec._internalName, QString());

    // restore current reference
    Reference::Fields fields = _currentRef.getAllFields();
    for(Reference::Fields::iterator it = fields.begin(); it != fields.end(); ++it)
    {
        QString fieldName  = it.key();
        QString fieldValue = it.value().toString();
        if(fieldName != "type" && !fieldValue.isEmpty())
            createField(fieldName, fieldValue);
    }

    // add the fields
    foreach(const Field& field, _fields)
    {
        int lastRow = _layout->rowCount();
        _layout->addWidget(field.first,  lastRow, 0);
        _layout->addWidget(field.second, lastRow, 1);
    }

    // add the spacer
    _layout->addItem(_spacer, _layout->rowCount(), 0);
}

void RefDlg::createField(const QString& fieldName, const QString& fieldValue)
{
    // create a field
    QLabel*  label = new QLabel(fieldName,  this);
    QWidget* edit  = createEdit(fieldName, fieldValue);

    // replace existing
    Fields::iterator it = _fields.begin();
    for(; it != _fields.end(); ++it)
    {
        if(it->first->text() == fieldName)
        {
            delete it->second;
            it->second = edit;
            return;
        }
    }

    // add as new
    _fields << Field(label, edit);
}

QWidget* RefDlg::createEdit(const QString& fieldName, const QString& fieldValue)
{
    if(fieldName == "abstract" || fieldName == "note")
        return new QPlainTextEdit(fieldValue, this);
    return new QLineEdit(fieldValue, this);
}

void RefDlg::clearFields()
{
    foreach(const Field& field, _fields)
    {
        delete field.first;
        delete field.second;
    }
    _fields.clear();
    _layout->removeItem(_spacer);
}

QString RefDlg::getFieldText(const Field& field) const
{
    QString fieldName = field.first->text();
    if(fieldName == "abstract" || fieldName == "note")
        return static_cast<QPlainTextEdit*>(field.second)->toPlainText();
    return static_cast<QLineEdit*>(field.second)->text();
}

Reference RefDlg::getReference() const
{
    Reference result;
    result.setValue("type", _comboType->currentText());
    foreach(const Field& field, _fields)
        result.setValue(field.first->text(), getFieldText(field));
    return result;
}
