#include "FieldEdit.h"

void MyLineEdit::highlight(const QColor& color)
{
    QPalette palette = QLineEdit::palette();
    palette.setBrush(QPalette::Base, color);
    QLineEdit::setPalette(palette);
}

void MyPlainTextEdit::highlight(const QColor &color)
{
    QPalette palette = QPlainTextEdit::palette();
    palette.setBrush(QPalette::Base, color);
    QPlainTextEdit::setPalette(palette);
}
