#ifndef RELATEDPHRASEWIDGET_H
#define RELATEDPHRASEWIDGET_H

#include "ui_RelatedPhraseWidget.h"
#include "Common.h"
#include <QStandardItemModel>

class Thesaurus;
class RelatedPhraseModel;
class RelatedPhraseWidget : public QWidget
{
    Q_OBJECT
    
public:
    RelatedPhraseWidget(QWidget *parent = 0);
    void setCentralPhraseID(int id);

private slots:
    void onTagThesaurus   (const QStringList& relatedTags);
    void onPhraseThesaurus(const QStringList &relatedPhrases);
    void onRelatedDoubleClicked(const QModelIndex& idx);

signals:
    void doubleClicked(int phraseID);

private:
    void sort();

private:
    Ui::RelatedPhraseWidget ui;
    int centralPhraseID;
    RelatedPhraseModel* model;
    Thesaurus* tagThesaurus;
    Thesaurus* phraseThesaurus;
};

class RelatedPhraseModel : public QStandardItemModel
{
public:
    RelatedPhraseModel(QObject* parent = 0);
    void updateRecord(const Phrase& phrase, int proximity);

private:
    int findRecord(int id);

public:
    enum {COL_ID, COL_PHRASE, COL_EXPLANATION, COL_PROXIMITY};
};

#endif // RELATEDPHRASEWIDGET_H
