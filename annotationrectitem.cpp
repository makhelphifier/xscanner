#include "annotationrectitem.h"
#include <QPen>

AnnotationRectItem::AnnotationRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
    : QGraphicsRectItem(x, y, width, height, parent)
{
    // 设置画笔样式（黄色边框）
    QPen pen;
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    setPen(pen);

    // 设置填充为透明
    setBrush(Qt::transparent);

    // 设置标志，使其可被选中和拖动
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}
