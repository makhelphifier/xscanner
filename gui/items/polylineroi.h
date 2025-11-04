#ifndef POLYLINEROI_H
#define POLYLINEROI_H

#include "roi.h"
#include <QList>
#include <QPointF>
#include <QPainterPath>

class Handle;

/**
 * @brief 一个由多个可拖动句柄（顶点）定义的多线段ROI。
 */
class PolylineROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数: 用于从点列表创建
    explicit PolylineROI(const QList<QPointF>& points, QGraphicsItem* parent = nullptr);
    virtual ~PolylineROI() override = default;

    enum { Type = UserType + 9 }; // +8 已被 PointMeasureItem 使用
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // --- ROI 重写 ---
    void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true) override;
    void removeHandle(Handle* handle) override;

private:
    /**
     * @brief 根据 m_vertices 列表重新计算路径、形状和包围盒
     */
    void updatePath();

    QList<Handle*> m_vertices;       // 按顺序存储的顶点句柄
    mutable QPainterPath m_shapeCache; // 缓存的形状（用于点击检测）
    mutable QRectF m_boundingRectCache;  // 缓存的包围盒
    QPainterPath m_visualPath;     // 缓存的视觉路径（用于绘制）
};

#endif // POLYLINEROI_H
