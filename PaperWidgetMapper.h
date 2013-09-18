#ifndef PAPERWIDGETMAPPER_H
#define PAPERWIDGETMAPPER_H

#include <QDataWidgetMapper>

class AttachmentsWidget;
class RelatedPapersWidget;
class CoauthoredPapersWidget;

class PaperWidgetMapper : public QDataWidgetMapper
{
    Q_OBJECT

public:
    PaperWidgetMapper(QObject* parent = 0);
    void setAttachmentWidget(AttachmentsWidget*      widget);
    void setRelatedWidget   (RelatedPapersWidget*    widget);
    void setCoauthoredWidget(CoauthoredPapersWidget* widget);

    void setCurrentIndex(int index);

private:
    AttachmentsWidget*      _widgetAttachment;
    RelatedPapersWidget*    _widgetRelated;
    CoauthoredPapersWidget* _widgetCoauthored;
};

#endif // PAPERWIDGETMAPPER_H
