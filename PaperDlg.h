#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"

class Reference;
class TypeSpec;

// add/edit paper
class PaperDlg : public QDialog
{
	Q_OBJECT

public:
	PaperDlg(QWidget *parent = 0);
	virtual void accept();

    Reference getReference() const;
    void      setReference(const Reference& ref);

private slots:
    void onTypeChanged(const QString& typeName);  // highlight required fields of the type

private:
    void setTitle(const QString& title);
    void setType (const QString& type);
    void highlightField(QWidget* field, const QColor& color);

private:
	Ui::PaperDlgClass ui;
};

#endif // PAPERDLG_H
