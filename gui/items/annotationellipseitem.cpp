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

// 添加新的构造函数实现
AnnotationEllipseItem::AnnotationEllipseItem(const QPointF &startPoint, const QPointF &endPoint, QGraphicsItem *parent)
    : QGraphicsEllipseItem(QRectF(startPoint, endPoint).normalized(), parent)
{
    // 复用现有设置
    QPen pen;
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    setPen(pen);
    setBrush(Qt::transparent);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}
