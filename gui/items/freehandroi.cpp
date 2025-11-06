#include "freehandroi.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>

FreehandROI::FreehandROI(const QPainterPath& path, QGraphicsItem* parent)
    : ROI(path.boundingRect().topLeft(), path.boundingRect().size(), parent),
    m_path(path)
{
    setFlag(QGraphicsItem::ItemIsMovable, false);
    QPointF offset = path.boundingRect().topLeft();
    setPos(offset);
    m_path.translate(-offset);
    QPainterPathStroker stroker;
    stroker.setWidth(10.0);
    m_shapeCache = stroker.createStroke(m_path);
    setPen(QPen(Qt::yellow, 2));
}

void FreehandROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen p = m_currentPen;
    p.setWidth(2);
    painter->setPen(p);

    painter->drawPath(m_path);
}

QRectF FreehandROI::boundingRect() const
{
    return m_path.boundingRect();
}

QPainterPath FreehandROI::shape() const
{
    return m_shapeCache;
}

void FreehandROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    Q_UNUSED(handle);
    Q_UNUSED(scenePos);
    Q_UNUSED(finish);
}
