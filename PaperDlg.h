#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class Reference;
class TypeSpec;
class RefSpec;

// A dlg for adding/editing a paper
// All fields except type and tags are stored in _fields,
// because they are all FieldEdit, and are manupulated in the same way

// Note abstract and note use MyPlainTextEdit, which may differ a little from the others
// authors and editors are QStringList, and need special treatment
// tags are stored in separated table, not in the reference

// setTitle() fixes title automatically (for protection and case errors)
// setType() will guess the type from publication if it's not valid; call it after setting publication

class PaperDlg : public QDialog
{
	Q_OBJECT

    typedef QPair<QString, FieldEdit*> Field;    // fieldName and edit
    typedef QList<Field>               Fields;

public:
    PaperDlg(QWidget* parent = 0);
	virtual void accept();

    Reference getReference() const;
    void      setReference(const Reference& ref);

private slots:
    void onTypeChanged(const QString& typeName);  // highlight required fields of the type

private:
    void setTitle(const QString& title);  // will fix errors in the title
    void setType (const QString& type);   // if type invalide, guess it from the publication

private:
	Ui::PaperDlgClass ui;
    Fields   _fields;  // stores all the fields except type and tags
    RefSpec* _spec;    // bibtex spec for highlighing required fields and loading types
};

#endif // PAPERDLG_H
