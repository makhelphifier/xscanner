#ifndef ANNOTATIONLINEITEM_H
#define ANNOTATIONLINEITEM_H

#include <QGraphicsLineItem>

class AnnotationLineItem : public QGraphicsLineItem
{
public:
    // 构造函数，传入线的起点和终点坐标
    AnnotationLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = nullptr);
};

#endif // ANNOTATIONLINEITEM_H
