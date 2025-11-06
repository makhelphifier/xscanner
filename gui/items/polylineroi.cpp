#include "polylineroi.h"
#include "handle.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QDebug>

PolylineROI::PolylineROI(const QList<QPointF>& points, QGraphicsItem* parent)
    : ROI(QPointF(0,0), QSizeF(1,1), parent)
{
    if (points.size() < 2) {
        qWarning() << "PolylineROI created with less than 2 points.";
    }

    // 1. 为每个点创建自由句柄
    for (const QPointF& p : points) {
        Handle* handle = addFreeHandle(p);
        m_vertices.append(handle);
    }

    // 2. 不允许拖动ROI本体（只能拖动句柄）
    setFlag(QGraphicsItem::ItemIsMovable, false);

    // 3. 初始计算路径和包围盒
    updatePath();
}

void PolylineROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_vertices.size() < 2) return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen p = m_currentPen;
    p.setWidth(2);
    painter->setPen(p);

    painter->drawPath(m_visualPath);
}

QRectF PolylineROI::boundingRect() const
{
    return m_boundingRectCache;
}

QPainterPath PolylineROI::shape() const
{
    return m_shapeCache;
}

void PolylineROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    ROI::movePoint(handle, scenePos, finish);
    updatePath();
    update();
}

void PolylineROI::removeHandle(Handle* handle)
{
    m_vertices.removeAll(handle);
    ROI::removeHandle(handle);
    updatePath();
    update();
}


void PolylineROI::updatePath()
{
    prepareGeometryChange();

    QPainterPath visualPath;
    QPainterPath shapePath;

    if (m_vertices.size() < 2) {
        // 没有足够的点来画线
        m_visualPath = visualPath;
        m_shapeCache = shapePath;
        m_boundingRectCache = QRectF();
        return;
    }

    // 1. 创建视觉路径 (Visual Path)
    visualPath.moveTo(m_vertices.first()->pos());
    for (int i = 1; i < m_vertices.size(); ++i) {
        visualPath.lineTo(m_vertices[i]->pos());
    }
    m_visualPath = visualPath;

    // 2. 创建形状路径 (Shape Path)
    QPainterPathStroker stroker;
    stroker.setWidth(10.0); // 10px 的点击宽度
    shapePath = stroker.createStroke(visualPath);

    // 3. 将所有手柄的形状添加到点击区域
    for (Handle* handle : qAsConst(m_vertices)) {
        // mapToParent(shape()) 将句柄的局部形状转换到ROI的坐标系
        shapePath.addPath(handle->mapToParent(handle->shape()));
    }

    // 4. 缓存结果
    m_shapeCache = shapePath;
    m_boundingRectCache = m_shapeCache.boundingRect();
}
