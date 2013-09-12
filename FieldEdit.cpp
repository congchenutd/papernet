#include "FieldEdit.h"

void FieldLineEdit::highlight(const QColor& color)
{
    QPalette palette = QLineEdit::palette();
    palette.setBrush(QPalette::Base, color);
    QLineEdit::setPalette(palette);
}

void FieldPlainTextEdit::highlight(const QColor &color)
{
    QPalette palette = QPlainTextEdit::palette();
    palette.setBrush(QPalette::Base, color);
    QPlainTextEdit::setPalette(palette);
}
