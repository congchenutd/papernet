#ifndef RELATEDPHRASEWIDGET_H
#define RELATEDPHRASEWIDGET_H

#include "ui_RelatedPhraseWidget.h"
#include <QSqlQueryModel>

class RelatedPhraseWidget : public QWidget
{
    Q_OBJECT
    
public:
    RelatedPhraseWidget(QWidget *parent = 0);
    void setCentralPhraseID(int id);
    
private:
    Ui::RelatedPhraseWidget ui;
    QSqlQueryModel model;
};

#endif // RELATEDPHRASEWIDGET_H
