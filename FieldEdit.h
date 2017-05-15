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

// adapters
class LineEditAdapter : public QLineEdit, public IFieldEdit
{
public:
    LineEditAdapter(QWidget* parent) : QLineEdit(parent) {}

    QString text() const;
    void setText(const QString& text);
    void highlight(const QColor& color);

    void setSeparator(const QString& separator);

private:
    QString formatSeparation(const QString& text) const;

private:
    QString _separator;
};

class PlainTextEditAdapter : public QPlainTextEdit, public IFieldEdit
{
public:
    PlainTextEditAdapter(QWidget* parent) : QPlainTextEdit(parent) {}

    QString text() const { return toPlainText(); }
    void setText(const QString& text) { setPlainText(text); }
    void highlight(const QColor& color);
};

class ComboBoxAdapter : public QComboBox, public IFieldEdit
{
public:
    ComboBoxAdapter(QWidget* parent = 0) : QComboBox(parent) {}

    QString text() const { return currentText(); }
    void setText(const QString& text) { setCurrentText(text); }
    void highlight(const QColor&) {}
};

#endif // FIELDEDIT_H
