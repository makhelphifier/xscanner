#include "annotationlineitem.h"
#include <QPen>

AnnotationLineItem::AnnotationLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent)
    : QGraphicsLineItem(x1, y1, x2, y2, parent)
{
    // 1. 设置画笔样式，使其在医学图像上更显眼
    // 通常使用黄色或亮绿色
    QPen pen;
    pen.setColor(Qt::red); // 设置颜色为黄色
    pen.setWidth(2);          // 设置线宽为2像素
    setPen(pen);

    // 2. 设置Item的标志，使其可被选中和拖动
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}
