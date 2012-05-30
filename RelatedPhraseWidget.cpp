#include "RelatedPhraseWidget.h"
#include "Thesaurus.h"
#include <QSqlQuery>

RelatedPhraseWidget::RelatedPhraseWidget(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
    centralPhraseID = -1;

    model = new RelatedPhraseModel(this);
    ui.tableView->setModel(model);
    ui.tableView->hideColumn(RelatedPhraseModel::COL_ID);
    ui.tableView->hideColumn(RelatedPhraseModel::COL_PROXIMITY);

    tagThesaurus    = new BigHugeThesaurus(this);
    phraseThesaurus = new BigHugeThesaurus(this);
    connect(tagThesaurus,    SIGNAL(response(QStringList)), this, SLOT(onTagThesaurus   (QStringList)));
    connect(phraseThesaurus, SIGNAL(response(QStringList)), this, SLOT(onPhraseThesaurus(QStringList)));
    connect(ui.tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRelatedDoubleClicked(QModelIndex)));
}

void RelatedPhraseWidget::setCentralPhraseID(int id)
{
    if(id < 0)
        return;
    centralPhraseID = id;
    model->removeRows(0, model->rowCount());

    // ------------ calculate proximity by tags -------------
    QSqlDatabase::database().transaction();

    // gather related tags: tags this phrase has (direct), and their proximate tags (from tagThesaurus)
    QSqlQuery query;    // query direct tags' names
    query.exec(tr("select Name from DictionaryTags, PhraseTag \
                   where Phrase = %1 and ID = Tag").arg(centralPhraseID));
    while(query.next())
       tagThesaurus->request(query.value(0).toString());   // query proximate tags with this direct tag

    // calculate proximity by direct tags:
    // 1. find in PhraseTag all direct tags
    // 2. count the # of all other phrases that have these tags
    // 3. associate the # with the phrases
    query.exec(tr("select Dictionary.ID, Dictionary.Phrase, \
                          Dictionary.Explanation, count(PhraseTag.Phrase) Proximity \
                  from Dictionary, PhraseTag \
                  where Tag in (select Tag from PhraseTag where Phrase = %1) \
                        and ID != %1 and ID = PhraseTag.Phrase \
                  group by PhraseTag.Phrase").arg(centralPhraseID));

    QSqlDatabase::database().commit();

    // save proximity
    while(query.next()) {
            model->updateRecord(Phrase(query.value(0).toInt(),
                                       query.value(1).toString(),
									   query.value(2).toString()),
									   query.value(3).toInt());
    }

    // -------------- update proximity by proximate phrases ---------------
    phraseThesaurus->request(findPhrase(centralPhraseID).name);

    sort();
}

// thesaurus returns related phrases
void RelatedPhraseWidget::onPhraseThesaurus(const QStringList& relatedPhrases)
{
    if(relatedPhrases.isEmpty())
        return;

    foreach(QString related, relatedPhrases)
        model->updateRecord(findPhrase(related), 1);

    sort();
}

// thesaurus returns related tags
void RelatedPhraseWidget::onTagThesaurus(const QStringList& relatedTags)
{
    if(relatedTags.isEmpty())
        return;

    QSqlDatabase::database().transaction();

    // get the IDs of relatedTags
    QStringList tagIDs;
    foreach(QString tagName, relatedTags)
    {
        int tagID = getTagID("DictionaryTags", tagName);
        if(tagID > 0)
            tagIDs << QString::number(tagID);
    }

    // calculate proximity by proximate tags:
    // 1. count the # of all other phrases that have these tags
    // 2. associate the # with the phrases
    QSqlQuery query;
    query.exec(tr("select Dictionary.ID, count(PhraseTag.Phrase) Proximity \
                  from Dictionary, PhraseTag \
                  where Tag in (%1) \
                  and ID != %2 and ID = PhraseTag.Phrase \
                  group by PhraseTag.Phrase").arg(tagIDs.join(",")).arg(centralPhraseID));

    QSqlDatabase::database().commit();

    // update proximity
    while(query.next())
        model->updateRecord(findPhrase(query.value(0).toInt()), 1);

    sort();
}

void RelatedPhraseWidget::sort()
{
    ui.tableView->sortByColumn(RelatedPhraseModel::COL_PHRASE,    Qt::AscendingOrder);
    ui.tableView->sortByColumn(RelatedPhraseModel::COL_PROXIMITY, Qt::DescendingOrder);
    ui.tableView->resizeColumnToContents(RelatedPhraseModel::COL_PHRASE);
}

void RelatedPhraseWidget::onRelatedDoubleClicked(const QModelIndex& idx) {
    emit doubleClicked(model->data(model->index(idx.row(), RelatedPhraseModel::COL_ID)).toInt());
}

///////////////////////////////////////////////////////////////////
RelatedPhraseModel::RelatedPhraseModel(QObject* parent)
    : QStandardItemModel(parent)
{
    setColumnCount(4);
    setHeaderData(COL_ID,          Qt::Horizontal, tr("ID"));
    setHeaderData(COL_PHRASE,      Qt::Horizontal, tr("Phrase"));
    setHeaderData(COL_EXPLANATION, Qt::Horizontal, tr("Explanation"));
    setHeaderData(COL_PROXIMITY,   Qt::Horizontal, tr("Proximity"));
}

void RelatedPhraseModel::updateRecord(const Phrase& phrase, int proximity)
{
    if(phrase.id < 0)
        return;

    int originalProximity;
    int row = findRecord(phrase.id);
    if(row < 0)
    {
        row = rowCount();
        insertRow(row);
        originalProximity = 0;
    }
    else {
        originalProximity = data(index(row, COL_PROXIMITY)).toInt();
    }
    setData(index(row, COL_ID),          phrase.id);
    setData(index(row, COL_PHRASE),      phrase.name);
    setData(index(row, COL_EXPLANATION), phrase.explanation);
    setData(index(row, COL_PROXIMITY),   proximity + originalProximity);
}

int RelatedPhraseModel::findRecord(int id)
{
    QModelIndexList indexes = match(index(0, COL_ID), Qt::DisplayRole, id, 1, Qt::MatchExactly);
    return indexes.isEmpty() ? -1 : indexes.front().row();
}
