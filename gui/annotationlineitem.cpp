#include "annotationlineitem.h"
#include <QPen>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QDebug>

void AnnotationLineItem::setupPen()
{
    QPen pen;
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    setPen(pen);
}

AnnotationLineItem::AnnotationLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent)
    : QGraphicsLineItem(x1, y1, x2, y2, parent)
{
    setupPen();
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

AnnotationLineItem::AnnotationLineItem(const QPointF &startPoint, QGraphicsItem *parent)
    : QGraphicsLineItem(QLineF(startPoint, startPoint), parent),
    m_drawing(true),
    m_startPoint(startPoint)
{
    setupPen();
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptHoverEvents(true);
}

void AnnotationLineItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_drawing) {
        QLineF newLine(m_startPoint, event->scenePos());
        setLine(newLine);
        event->accept();
        return;
    }
    QGraphicsLineItem::hoverMoveEvent(event);
}

void AnnotationLineItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_drawing && event->button() == Qt::LeftButton) {
        m_drawing = false;
        QLineF finalLine(m_startPoint, event->scenePos());
        setLine(finalLine);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setAcceptHoverEvents(false);
        event->accept();
        return;
    }
    QGraphicsLineItem::mousePressEvent(event);
}
