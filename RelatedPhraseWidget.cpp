#include "RelatedPhraseWidget.h"

RelatedPhraseWidget::RelatedPhraseWidget(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
}

void RelatedPhraseWidget::setCentralPhraseID(int id)
{
    if(id < 0)
        return;
}
