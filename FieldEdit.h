#ifndef FIELDEDIT_H
#define FIELDEDIT_H

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>

// an interface for field edit, used by PaperDlg
class IFieldEdit
{
public:
    virtual ~IFieldEdit() {}
    virtual QString text() const = 0;
    virtual void setText  (const QString& text) = 0;
    virtual void highlight(const QColor& color) = 0;
};

// two edits, all operations delegated to QLineEdit/QPlainTextEdit
class FieldLineEdit : public QLineEdit, public IFieldEdit
{
public:
    FieldLineEdit(QWidget* parent) : QLineEdit(parent) {}

    virtual QString text() const { return QLineEdit::text(); }
    virtual void setText(const QString& text) { QLineEdit::setText(text); }
    virtual void highlight(const QColor& color);
};

class FieldPlainTextEdit : public QPlainTextEdit, public IFieldEdit
{
public:
    FieldPlainTextEdit(QWidget* parent) : QPlainTextEdit(parent) {}

    virtual QString text() const { return toPlainText(); }
    virtual void setText(const QString& text) { setPlainText(text); }
    virtual void highlight(const QColor& color);
};

class FieldComboBox : public QComboBox, public IFieldEdit
{
public:
	FieldComboBox(QWidget* parent = 0) : QComboBox(parent) {}

	virtual QString text() const { return currentText(); }
	virtual void setText(const QString& text) { setCurrentText(text); }
	virtual void highlight(const QColor&) {}
};

#endif // FIELDEDIT_H
