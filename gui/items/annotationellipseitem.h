#ifndef ANNOTATIONELLIPSEITEM_H
#define ANNOTATIONELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include <QPen>

class AnnotationEllipseItem : public QGraphicsEllipseItem
{
public:
    // 构造函数1: 用于最终创建
    AnnotationEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    // 构造函数2: 用于 GenericDrawingState 从起点和终点创建
    AnnotationEllipseItem(const QPointF &startPoint, const QPointF &endPoint, QGraphicsItem *parent = nullptr);


    // --- 为 GenericDrawingState 提供的信息 ---
    // 1. 定义预览时使用的图形项类型
    using PreviewItemType = QGraphicsEllipseItem;

    // 2. 定义如何初始化预览项的静态方法
    static void initPreview(PreviewItemType* item) {
        QPen pen(Qt::yellow, 1, Qt::DashLine);
        item->setPen(pen);
        item->setBrush(Qt::transparent);
        item->setZValue(1000);
    }

    // 3. 定义如何根据起点和当前点更新预览的静态方法
    static void updatePreview(PreviewItemType* item, const QPointF& startPoint, const QPointF& currentPoint) {
        item->setRect(QRectF(startPoint, currentPoint).normalized());
    }
};

#endif // ANNOTATIONELLIPSEITEM_H
