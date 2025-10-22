#include "annotationellipseitem.h"
#include <QPen>

AnnotationEllipseItem::AnnotationEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent)
{
    QPen pen;
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    setPen(pen);
    setBrush(Qt::transparent);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}
