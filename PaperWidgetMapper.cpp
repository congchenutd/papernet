#include "PaperWidgetMapper.h"
#include "Common.h"
#include "AttachmentsWidget.h"
#include "RelatedPapersWidget.h"
#include "CoauthoredPapersWidget.h"

PaperWidgetMapper::PaperWidgetMapper(QObject* parent) :
    QDataWidgetMapper(parent),
    _widgetAttachment(0),
    _widgetRelated(0),
    _widgetCoauthored(0)
{
}

void PaperWidgetMapper::setAttachmentWidget(AttachmentsWidget* widget) {
    if(widget != 0)
        _widgetAttachment = widget;
}

void PaperWidgetMapper::setRelatedWidget(RelatedPapersWidget* widget) {
    if(widget != 0)
        _widgetRelated = widget;
}

void PaperWidgetMapper::setCoauthoredWidget(CoauthoredPapersWidget* widget) {
    if(widget != 0)
        _widgetCoauthored = widget;
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
}
