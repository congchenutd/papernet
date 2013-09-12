#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class Reference;
class TypeSpec;

/////////////////////////////////////////////////////////////////////
/// A dlg for adding/editing a paper
/// All fields except tags are stored in _fields,
/// because they are all FieldEdit, and are manipulated in the same way
///
/// Note: abstract and note use MyPlainTextEdit, which may differ a little from the others
/// authors and editors are QStringList, and need special treatment
/// tags are stored in a separate table, not in the reference

/// setTitle() fixes title automatically (for protection and case errors)
/// setType() will guess the type from publication if it's not valid;
/// call it after setting publication
///
class PaperDlg : public QDialog
{
	Q_OBJECT

    typedef QPair<QString, IFieldEdit*> Field;    // fieldName -> edit
    typedef QList<Field>                Fields;

public:
    PaperDlg(QWidget* parent = 0);
	virtual void accept();

    Reference getReference() const;
    void      setReference(const Reference& ref);

    QString getPDFPath() const;
    void    setPDFPath(const QString& path);

    void showMergeMark();

private slots:
    void onTypeChanged(const QString& typeName);  // highlight required fields of the type
    void onGoogle();
    void onGotoPaper();
    void onAddPDF();

signals:
    void gotoPaper(int id);    // Select paper button clicked: select current paper in the view

private:
    void setTitle(const QString& title);  // may fix errors in the title
    void setType (const QString& type);   // if type invalide, guess it from the publication

private:
	Ui::PaperDlgClass ui;
    int      _id;      // paper id
    Fields   _fields;  // stores all the fields except type and tags
};

#endif // PAPERDLG_H
