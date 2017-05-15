#include "PaperWidgetMapper.h"
#include "Common.h"
#include "AttachmentsWidget.h"
#include "RelatedPapersWidget.h"
#include "CoauthoredPapersWidget.h"
#include "QuotesWidget.h"
#include "TagsWidget.h"

PaperWidgetMapper::PaperWidgetMapper(QObject* parent) :
    QDataWidgetMapper(parent),
    _widgetAttachment(0),
    _widgetRelated(0),
    _widgetCoauthored(0),
    _widgetQuotes(0),
    _widgetTags(0)
{}

void PaperWidgetMapper::setAttachmentWidget(AttachmentsWidget* widget) {
    _widgetAttachment = widget;
}
void PaperWidgetMapper::setRelatedWidget(RelatedPapersWidget* widget) {
    _widgetRelated = widget;
}
void PaperWidgetMapper::setCoauthoredWidget(CoauthoredPapersWidget* widget) {
    _widgetCoauthored = widget;
}
void PaperWidgetMapper::setQuotesWidget(QuotesWidget* widget) {
    _widgetQuotes = widget;
}
void PaperWidgetMapper::setTagsWidget(TagsWidget* widget) {
    _widgetTags = widget;
}

void PaperWidgetMapper::setCurrentIndex(int row)
{
    QDataWidgetMapper::setCurrentIndex(row);

    int paperID = model()->data(model()->index(row, PAPER_ID)).toInt();

    if(_widgetAttachment != 0)
        _widgetAttachment->setPaper(paperID);

    if(_widgetRelated != 0)
        _widgetRelated->setCentralPaper(paperID);

    if(_widgetCoauthored != 0)
        _widgetCoauthored->setCentralPaper(paperID);

    if(_widgetQuotes != 0)
        _widgetQuotes->setCentralPaper(paperID);

    if(_widgetTags != 0)
        _widgetTags->highLight(getTagsOfPaper(paperID));
}

void PaperWidgetMapper::refresh() {
    setCurrentIndex(currentIndex());
}
