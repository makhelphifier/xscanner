#include "annotationpointitem.h"
#include <QPen>
#include <QBrush>
#include <QGraphicsPixmapItem>

const qreal POINT_RADIUS = 5;

AnnotationPointItem::AnnotationPointItem(qreal x, qreal y, QGraphicsPixmapItem *imageItem, const QImage &image, QGraphicsItem *parent)
    : QGraphicsEllipseItem(x - POINT_RADIUS, y - POINT_RADIUS, POINT_RADIUS * 2, POINT_RADIUS * 2, parent),
    m_imageItem(imageItem),
    m_image(image)
{
    QPen pen;
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    setPen(pen);

    setBrush(Qt::yellow);

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setDefaultTextColor(Qt::yellow);
    m_textItem->setPos(x + POINT_RADIUS, y - m_textItem->boundingRect().height() / 2);

}

void AnnotationPointItem::setMeasurement(const QPointF &pos, int grayValue)
{
    // 格式化要显示的字符串
    QString text = QString("X: %1\nY: %2\nVal: %3")
                       .arg(pos.x(), 0, 'f', 1)
                       .arg(pos.y(), 0, 'f', 1)
                       .arg(grayValue);
    m_textItem->setPlainText(text);
}

QVariant AnnotationPointItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange && scene()) {
        QPointF newScenePos = value.toPointF();

        QPointF newCenterPos = value.toPointF() + rect().center();

        if (m_imageItem->contains(newCenterPos)) {
            QPointF imagePos = m_imageItem->mapFromScene(newCenterPos);
            int x = qRound(imagePos.x());
            int y = qRound(imagePos.y());

            if (m_image.valid(x, y)) {
                int grayValue = qGray(m_image.pixel(x, y));
                setMeasurement(imagePos, grayValue);
            }
        }
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

