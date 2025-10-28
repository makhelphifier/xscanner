// rectroi.h

#ifndef RECTROI_H
#define RECTROI_H

#include "roi.h"
#include <QGraphicsRectItem> // 用于预览项
#include <QPen>

/**
 * @brief 一个预设了缩放句柄的矩形ROI子类。
 *
 * 这个类继承自ROI，并在构造时自动添加了常用的Scale Handles。
 * 它适配了 GenericDrawingState，因此可以被交互式地绘制。
 */
class RectROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数1: 用于代码直接创建
    explicit RectROI(const QPointF& pos, const QSizeF& size, bool centered = false, bool sideScalers = false, QGraphicsItem* parent = nullptr);

    // 构造函数2: 用于 GenericDrawingState 从起点和终点创建
    explicit RectROI(const QPointF& startPoint, const QPointF& endPoint, QGraphicsItem* parent = nullptr);

    virtual ~RectROI() override = default;

    enum { Type = UserType + 3 };
    int type() const override { return Type; }

    // --- 为 GenericDrawingState 提供的适配接口 ---

    // 1. 定义预览时使用的图形项类型
    using PreviewItemType = QGraphicsRectItem;

    // 2. 定义如何初始化预览项的静态方法
    static void initPreview(PreviewItemType* item) {
        QPen pen(Qt::yellow, 1, Qt::DashLine);
        item->setPen(pen);
        item->setBrush(Qt::transparent);
        item->setZValue(1000); // 确保预览在最上层
    }

    // 3. 定义如何根据起点和当前点更新预览的静态方法
    static void updatePreview(PreviewItemType* item, const QPointF& startPoint, const QPointF& currentPoint) {
        item->setRect(QRectF(startPoint, currentPoint).normalized());
    }
};

#endif // RECTROI_H
