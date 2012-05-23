#include "RelatedPhraseWidget.h"

RelatedPhraseWidget::RelatedPhraseWidget(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
	ui.tableView->setModel(&model);
}

void RelatedPhraseWidget::setCentralPhraseID(int id)
{
    if(id < 0)
        return;

	model.setQuery(tr("select Phrase, Explanation from Dictionary where ID in \
							(select Phrase from PhraseTag where Tag in \
								(select Tag from PhraseTag where Phrase = %1) \
							 and Phrase != %1) \
					  ").arg(id));
}
