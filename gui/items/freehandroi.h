#ifndef FREEHANDROI_H
#define FREEHANDROI_H

#include "roi.h"
#include <QPainterPath>

/**
 * @brief 一个用于显示自由绘制轨迹的简单ROI。
 *
 * 它只负责显示路径，不提供交互式句柄。
 */
class FreehandROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数: 从一个 QPainterPath 创建
    explicit FreehandROI(const QPainterPath& path, QGraphicsItem* parent = nullptr);
    virtual ~FreehandROI() override = default;

    enum { Type = UserType + 10 }; // 假设 PolylineROI 是 +9
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // --- ROI 重写 (禁用移动) ---
    void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true) override;

private:
    QPainterPath m_path; // 存储的轨迹
    mutable QPainterPath m_shapeCache; // 缓存的点击形状
};

#endif // FREEHANDROI_H
