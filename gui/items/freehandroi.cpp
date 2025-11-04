#include "freehandroi.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>

FreehandROI::FreehandROI(const QPainterPath& path, QGraphicsItem* parent)
    : ROI(path.boundingRect().topLeft(), path.boundingRect().size(), parent),
    m_path(path)
{
    // 1. 设置此ROI不可移动（因为没有句柄来更新它）
    setFlag(QGraphicsItem::ItemIsMovable, false);

    // 2. 将基类的 pos 设置为 (0,0)，并使路径相对于 (0,0)
    //    这能确保 boundingRect() 和 paint() 在同一个坐标系中
    QPointF offset = path.boundingRect().topLeft();
    setPos(offset);
    m_path.translate(-offset);

    // 3. 计算点击形状 (shape)
    QPainterPathStroker stroker;
    stroker.setWidth(10.0); // 10px 点击宽度
    m_shapeCache = stroker.createStroke(m_path);

    // 4. 设置默认画笔（来自基类）
    setPen(QPen(Qt::yellow, 2));
}

void FreehandROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);

    // m_currentPen 是从基类 ROI 继承的
    QPen p = m_currentPen;
    p.setWidth(2); // 确保是 2 像素
    painter->setPen(p);

    painter->drawPath(m_path);
}

QRectF FreehandROI::boundingRect() const
{
    // 返回路径的包围盒（已在构造函数中移至 0,0）
    return m_path.boundingRect();
}

QPainterPath FreehandROI::shape() const
{
    // 返回加粗的形状，以便于选中
    return m_shapeCache;
}

void FreehandROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    // 禁用所有句柄交互
    Q_UNUSED(handle);
    Q_UNUSED(scenePos);
    Q_UNUSED(finish);
    // 基类 ROI::movePoint(handle, scenePos, finish);
}
