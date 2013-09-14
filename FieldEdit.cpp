#include "FieldEdit.h"
#include "Common.h"

QString LineEditAdapter::text() const {
    return formatSeparation(QLineEdit::text());
}

void LineEditAdapter::setText(const QString& text)
{
    QLineEdit::setText(formatSeparation(text));
    setCursorPosition(0);   // ensure the left is shown
}

void LineEditAdapter::highlight(const QColor& color)
{
    QPalette palette = QLineEdit::palette();
    palette.setBrush(QPalette::Base, color);
    QLineEdit::setPalette(palette);
}

void LineEditAdapter::setSeparator(const QString& separator) {
    _separator = separator;
}

QString LineEditAdapter::formatSeparation(const QString& text) const {
    return _separator.isEmpty() ? text
                                : splitLine(text, _separator).join(_separator);
}


//////////////////////////////////////////////////////////////////////
void PlainTextEditAdapter::highlight(const QColor& color)
{
    QPalette palette = QPlainTextEdit::palette();
    palette.setBrush(QPalette::Base, color);
    QPlainTextEdit::setPalette(palette);
}
