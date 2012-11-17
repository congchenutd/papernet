#ifndef FIELDEDIT_H
#define FIELDEDIT_H

#include <QLineEdit>
#include <QPlainTextEdit>

class FieldEdit
{
public:
    virtual ~FieldEdit() {}
    virtual QString text() const = 0;
    virtual void setText(const QString& text) = 0;
    virtual void highlight(const QColor& color) = 0;
};

class MyLineEdit : public QLineEdit, public FieldEdit
{
public:
    MyLineEdit(QWidget* parent) : QLineEdit(parent) {}

    virtual QString text() const { return QLineEdit::text(); }
    virtual void setText(const QString& text) { QLineEdit::setText(text); }
    virtual void highlight(const QColor& color);
};

class MyPlainTextEdit : public QPlainTextEdit, public FieldEdit
{
public:
    MyPlainTextEdit(QWidget* parent) : QPlainTextEdit(parent) {}

    virtual QString text() const { return QPlainTextEdit::toPlainText(); }
    virtual void setText(const QString& text) { QPlainTextEdit::setPlainText(text); }
    virtual void highlight(const QColor& color);
};

#endif // FIELDEDIT_H
