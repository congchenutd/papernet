#ifndef PAPERWIDGETMAPPER_H
#define PAPERWIDGETMAPPER_H

#include <QDataWidgetMapper>

class AttachmentsWidget;
class RelatedPapersWidget;
class CoauthoredPapersWidget;
class QuotesWidget;
class TagsWidget;

// Maps items in the Papers table to paper-related widgets
class PaperWidgetMapper : public QDataWidgetMapper
{
    Q_OBJECT

public:
    PaperWidgetMapper(QObject* parent = 0);
    void setAttachmentWidget(AttachmentsWidget*      widget);
    void setRelatedWidget   (RelatedPapersWidget*    widget);
    void setCoauthoredWidget(CoauthoredPapersWidget* widget);
    void setQuotesWidget    (QuotesWidget*           widget);
    void setTagsWidget      (TagsWidget*             widget);
    void refresh();

    void setCurrentIndex(int index);

private:
    AttachmentsWidget*      _widgetAttachment;
    RelatedPapersWidget*    _widgetRelated;
    CoauthoredPapersWidget* _widgetCoauthored;
    QuotesWidget*           _widgetQuotes;
    TagsWidget*             _widgetTags;
};

#endif // PAPERWIDGETMAPPER_H
