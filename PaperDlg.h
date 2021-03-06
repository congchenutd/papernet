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

	void showMergeMark();

public slots:
	void onDirty();

private slots:
	void onTypeChanged(const QString& typeName);  // highlight required fields of the type
	void onGoogle();
	void onAddPDF();

private:
	void setTitle(const QString& title);  // may fix errors in the title
	void setType (const QString& type);   // if type invalide, guess it from the publication

	void enableDirtyConnections();
	void disableDirtyConnections();

private:
	Ui::PaperDlgClass ui;
	int      _id;      // paper id
	Fields   _fields;  // stores all the fields except type and tags
	bool     _dirty;
};

#endif // PAPERDLG_H
