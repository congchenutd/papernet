#ifndef REFDLG_H
#define REFDLG_H

#include "ui_RefDlg.h"
#include "Reference.h"

class RefSpec;
class SetTextComboBox;
class QLabel;
class QPlainTextEdit;

class RefDlg : public QDialog
{
    Q_OBJECT

    typedef QPair<QLabel*, QWidget*> Field;  // a label and a line edit/text edit
    typedef QList<Field>             Fields;
    
public:
    explicit RefDlg(QWidget* parent = 0);

    // 2 ways to init
    void setSource(const QString& bibtexSource);    // unparsed source
    void setReference(const Reference& reference);  // parsed

    Reference getReference() const;

private slots:
    void reloadFields(const QString& typeName);   // reload from spec and _currentRef

private:
    QWidget* createEdit (const QString& fieldName, const QString& fieldValue);
    void     createField(const QString& fieldName, const QString& fieldValue);
    void     clearFields();
    QString  getFieldText(const Field& field) const;
    
private:
    Ui::RefDlg ui;
    QGridLayout* _layout;
    SetTextComboBox* _comboType;
    Fields       _fields;
    Reference    _currentRef;
    QSpacerItem* _spacer;
    RefSpec*     _spec;      // bibtex spec used for _fields
};


#endif // REFDLG_H
