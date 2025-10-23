#ifndef ANNOTATIONRECTITEM_H
#define ANNOTATIONRECTITEM_H

#include <QGraphicsRectItem>

class AnnotationRectItem : public QGraphicsRectItem
{
public:
    AnnotationRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);
};

#endif // ANNOTATIONRECTITEM_H
