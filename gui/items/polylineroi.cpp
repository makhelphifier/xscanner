#include "polylineroi.h"
#include "handle.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QDebug>

PolylineROI::PolylineROI(const QList<QPointF>& points, QGraphicsItem* parent)
    : ROI(QPointF(0,0), QSizeF(1,1), parent) // 基类 pos/size 无意义
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
    QPen p = m_currentPen; // m_currentPen 来自基类 ROI
    p.setWidth(2);
    painter->setPen(p);

    // m_visualPath 是在 updatePath() 中计算好的
    painter->drawPath(m_visualPath);
}

QRectF PolylineROI::boundingRect() const
{
    return m_boundingRectCache;
}

QPainterPath PolylineROI::shape() const
{
    // m_shapeCache 是在 updatePath() 中计算好的
    return m_shapeCache;
}

void PolylineROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    // 1. 调用基类方法，让 FreeHandle 更新其内部位置
    ROI::movePoint(handle, scenePos, finish);

    // 2. 重新计算我们的路径
    updatePath();
    update(); // 触发重绘
}

void PolylineROI::removeHandle(Handle* handle)
{
    // 1. 从我们的顶点列表中移除
    m_vertices.removeAll(handle);

    // 2. 调用基类方法以正确删除句柄
    ROI::removeHandle(handle);

    // 3. 重新计算路径
    updatePath();
    update(); // 触发重绘
}


void PolylineROI::updatePath()
{
    // 通知 Qt 几何形状即将改变
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
