#ifndef ANNOTATIONELLIPSEITEM_H
#define ANNOTATIONELLIPSEITEM_H

#include <QGraphicsEllipseItem>

class AnnotationEllipseItem : public QGraphicsEllipseItem
{
public:
    AnnotationEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);
};

#endif // ANNOTATIONELLIPSEITEM_H
