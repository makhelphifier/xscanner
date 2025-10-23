#include "annotationverticallineitem.h"
#include <QPen>
#include <QPainter>
#include <QGraphicsSimpleTextItem>
#include <QFont>

AnnotationVerticalLineItem::AnnotationVerticalLineItem(qreal x, qreal sceneHeight, qreal initialScale, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_sceneHeight(sceneHeight), m_baseFontSize(60.0), m_currentScale(initialScale)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setPos(x, 0);

    m_label = new QGraphicsSimpleTextItem(this);
    m_label->setText(QString("x=%1").arg(x, 0, 'f', 2));
    m_label->setBrush(Qt::yellow);

    qreal newFontSize = m_baseFontSize / m_currentScale;
    QFont font("Arial");
    font.setPointSizeF(newFontSize);
    m_label->setFont(font);
}
void AnnotationVerticalLineItem::updateScale(qreal scale)
{
    if (qFuzzyCompare(m_currentScale, scale)) {
        return;
    }
    m_currentScale = scale;

    prepareGeometryChange();

    qreal newFontSize = m_baseFontSize / m_currentScale;

    QFont font("Arial");
    font.setPointSizeF(newFontSize);
    m_label->setFont(font);

    // 垂直线标签位置不变，只需更新字体
    update();
}


QRectF AnnotationVerticalLineItem::boundingRect() const
{
    QRectF labelRect = m_label->boundingRect().translated(m_label->pos());
    // 为线条的厚度提供一些水平空间
    QRectF lineRect(-2, 0, 4, m_sceneHeight);
    return labelRect.united(lineRect);
}

void AnnotationVerticalLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(Qt::yellow);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawLine(QPointF(0, 0), QPointF(0, m_sceneHeight));
}

QVariant AnnotationVerticalLineItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        newPos.setY(0); // 约束为只能水平移动
        m_label->setText(QString("x=%1").arg(newPos.x(), 0, 'f', 2));
        return newPos;
    }
    return QGraphicsObject::itemChange(change, value);
}
