#include "RefDlg.h"
#include "RefFormatSpec.h"
#include "RefParser.h"
#include <QGridLayout>
#include <QLineEdit>

RefDlg::RefDlg(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    _layout = new QGridLayout(this);
//    _layout->setSizeConstraint(QLayout::SetFixedSize);
    ui.tabRef->setLayout(_layout);
    _layout->addWidget(ui.labelType, 0, 0);
    _layout->addWidget(ui.comboType, 0, 1);
    _spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    connect(ui.comboType, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTypeChanged(QString)));
}

void RefDlg::setSource(const QString& bibtexSource)
{
    RefSpec* spec = RefSpecFactory::getInstance()->getSpec("bib");
    if(spec == 0)
        return;
    QList<Reference> refs = spec->getParser()->parse(bibtexSource, spec);
    if(!refs.isEmpty())
        setReference(refs.front());
}

void RefDlg::setReference(const Reference& reference)
{
    _currentRef = reference;
    QString typeName = reference.getValue("type").toString();
    ui.comboType->setCurrentText(typeName);  // triggeres onTypeChanged()
    onTypeChanged(typeName);
}

void RefDlg::restoreReference(const Reference& reference)
{
    Reference::Fields fields = reference.getAllFields();
    for(Reference::Fields::iterator it = fields.begin(); it != fields.end(); ++it)
    {
        QString fieldName  = it.key();
        QString fieldValue = it.value().toString();
        if(fieldName != "type")
        {
            int idx = findField(fieldName);
            if(idx > -1)
                _fields[idx].second->setText(fieldValue);
            else
                addField(fieldName, fieldValue);
        }
    }
    _layout->addItem(_spacer, _layout->rowCount(), 0);
}

void RefDlg::onTypeChanged(const QString& typeName)
{
    loadTypeSpec(typeName);
    restoreReference(_currentRef);
}

void RefDlg::addField(const QString& fieldName, const QString& fieldValue)
{
    int lastRow = _layout->rowCount();
    QLabel*    label = new QLabel   (fieldName,  this);
    QLineEdit* edit  = new QLineEdit(fieldValue, this);
    _layout->addWidget(label, lastRow, 0);
    _layout->addWidget(edit,  lastRow, 1);
    _fields << Field(label, edit);
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

int RefDlg::findField(const QString& fieldName) const
{
    for(int i = 0; i < _fields.size(); ++i)
        if(_fields[i].first->text() == fieldName)
            return i;
    return -1;
}

void RefDlg::loadTypeSpec(const QString& typeName)
{
    RefSpec* refSpec = RefSpecFactory::getInstance()->getSpec("bib");
    if(refSpec == 0)
        return;

    TypeSpec typeSpec = refSpec->getType(typeName);
    if(!typeSpec.isValid())
        return;

    clearFields();
    foreach(const FieldSpec& fieldSpec, typeSpec.getAllFields())
        addField(fieldSpec._internalName, QString());
}
