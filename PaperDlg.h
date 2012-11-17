#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class Reference;
class TypeSpec;
class RefSpec;

// add/edit paper
class PaperDlg : public QDialog
{
	Q_OBJECT

    typedef QPair<QString, FieldEdit*> Field;

public:
    PaperDlg(QWidget* parent = 0);
	virtual void accept();

    Reference getReference() const;
    void      setReference(const Reference& ref);

private slots:
    void onTypeChanged(const QString& typeName);  // highlight required fields of the type

private:
    void setTitle(const QString& title);
    void setType (const QString& type);

private:
	Ui::PaperDlgClass ui;
    QList<Field> _fields;
    RefSpec*     _spec;
};

#endif // PAPERDLG_H
