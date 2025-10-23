#include "annotationhorizontallineitem.h"
#include <QPen>
#include <QPainter>
#include <QGraphicsSimpleTextItem>
#include <QFont>

AnnotationHorizontalLineItem::AnnotationHorizontalLineItem(qreal y, qreal sceneWidth, qreal initialScale, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_sceneWidth(sceneWidth), m_baseFontSize(60.0), m_currentScale(initialScale)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setPos(0, y);

    m_label = new QGraphicsSimpleTextItem(this);
    m_label->setText(QString("y=%1").arg(y, 0, 'f', 2));
    m_label->setBrush(Qt::yellow);

    qreal newFontSize = m_baseFontSize / m_currentScale;
    QFont font("Arial");
    font.setPointSizeF(newFontSize);
    m_label->setFont(font);
    m_label->setPos(5, -m_label->boundingRect().height() - 2);
}

void AnnotationHorizontalLineItem::updateScale(qreal scale)
{
    if (qFuzzyCompare(m_currentScale, scale)) {
        return;
    }
    m_currentScale = scale;

    // 通知 QGraphicsView 元素的边界即将改变
    prepareGeometryChange();

    // 根据当前视图缩放比例反向计算字体大小
    qreal newFontSize = m_baseFontSize / m_currentScale;

    QFont font("Arial");
    font.setPointSizeF(newFontSize);
    m_label->setFont(font);

    // 字体大小变化后，重新定位标签以保持在左上角
    m_label->setPos(5, -m_label->boundingRect().height() - 2);

    update(); // 请求重绘
}
QRectF AnnotationHorizontalLineItem::boundingRect() const
{
    QRectF labelRect = m_label->boundingRect().translated(m_label->pos());
    // 为线条的厚度提供一些垂直空间
    QRectF lineRect(0, -2, m_sceneWidth, 4);
    return labelRect.united(lineRect);
}

void AnnotationHorizontalLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(Qt::yellow);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawLine(QPointF(0, 0), QPointF(m_sceneWidth, 0));
}

QVariant AnnotationHorizontalLineItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        newPos.setX(0); // 约束为只能垂直移动
        m_label->setText(QString("y=%1").arg(newPos.y(), 0, 'f', 2));
        return newPos;
    }
    return QGraphicsObject::itemChange(change, value);
}
