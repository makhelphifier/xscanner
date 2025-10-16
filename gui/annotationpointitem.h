#ifndef ANNOTATIONPOINTITEM_H
#define ANNOTATIONPOINTITEM_H

#include <QGraphicsEllipseItem>
#include <QImage>

class QGraphicsTextItem;
class QGraphicsPixmapItem;

class AnnotationPointItem : public QGraphicsEllipseItem
{
public:
    AnnotationPointItem(qreal x, qreal y, QGraphicsPixmapItem *imageItem, const QImage &image, QGraphicsItem *parent = nullptr);

    void setMeasurement(const QPointF &pos, int grayValue);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QGraphicsTextItem *m_textItem;
    QGraphicsPixmapItem *m_imageItem;
    const QImage &m_image;
};

#endif // ANNOTATIONPOINTITEM_H
