#include "infinitelineitem.h"
#include "gui/views/imageviewer.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include "infinitelinelabel.h"

InfiniteLineItem::InfiniteLineItem(ImageViewer* viewer, qreal angle, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_viewer(viewer), m_angle(0), m_label(nullptr)
{
    if (!m_viewer) {
        qWarning() << "InfiniteLineItem created with null viewer!";
    }

    setPen(QPen(Qt::green, 0)); // 0宽度画笔（随缩放保持1像素）
    setHoverPen(QPen(Qt::red, 0));
    m_currentPen = m_pen;

    setAngle(angle); // 设置角度和初始位置
    setValue(0);     // 默认在 (0,0)
    m_label = new InfiniteLineLabel(this);
    setAcceptHoverEvents(true);

    if (m_viewer) {
        connect(m_viewer, &ImageViewer::viewZoomed, this, &InfiniteLineItem::onViewChanged);
        connect(m_viewer, &ImageViewer::viewPanned, this, &InfiniteLineItem::onViewChanged);
    }
    QTimer::singleShot(0, m_label, &InfiniteLineLabel::onViewChanged);
}

void InfiniteLineItem::setPen(const QPen& pen)
{
    m_pen = pen;
    if (!isUnderMouse() && !m_isMoving) {
        m_currentPen = m_pen;
        update();
    }
}

void InfiniteLineItem::setHoverPen(const QPen& pen)
{
    m_hoverPen = pen;
    if (isUnderMouse() || m_isMoving) {
        m_currentPen = m_hoverPen;
        update();
    }
}
void InfiniteLineItem::setAngle(qreal angle)
{
    m_angle = angle;
    setRotation(m_angle);
    if (m_label) {
        m_label->setFormat(m_angle);
    }

    onViewChanged();
}

void InfiniteLineItem::setValue(qreal value)
{
    QPointF newPos;
    if (m_angle == 90) {
        newPos = QPointF(value, 0);
    } else if (m_angle == 0) { // 水平线
        newPos = QPointF(0, value);
    } else {
        newPos = QPointF(0, 0);
    }

    if (pos() == newPos) {
        return;
    }

    setPos(newPos);

    emit positionChanged(value);
}

qreal InfiniteLineItem::value() const
{
    if (m_angle == 90) {
        return pos().x();
    } else if (m_angle == 0) {
        return pos().y();
    }
    return 0;
}


void InfiniteLineItem::onViewChanged()
{
    if (m_cachedBoundingRect.isValid()) {
        prepareGeometryChange();
        m_cachedBoundingRect = QRectF();
    }
    if (m_label) {
        m_label->onViewChanged();
    }
}

QRectF InfiniteLineItem::boundingRect() const
{
    if (m_cachedBoundingRect.isValid()) {
        return m_cachedBoundingRect;
    }
    m_cachedBoundingRect = calculateBoundingRect();
    return m_cachedBoundingRect;
}

QRectF InfiniteLineItem::calculateBoundingRect() const
{
    if (m_cachedBoundingRect.isValid()) {
        return m_cachedBoundingRect;
    }
    if (!m_viewer) {
        m_cachedBoundingRect = QRectF(-1000, -5, 2000, 10); // 默认值
    } else {
        // 1. 获取 Viewport 在 Scene 中的矩形 (场景坐标系)
        QRectF sceneViewRect = m_viewer->mapToScene(m_viewer->viewport()->rect()).boundingRect();

        // 2. 将 Scene 矩形 转换到 Item 的局部坐标系
        QRectF localViewRect = mapFromScene(sceneViewRect).boundingRect();

        qreal penWidth = m_currentPen.widthF();
        if (penWidth == 0) penWidth = 1.0; // 0宽度笔刷实际是1像素
        qreal padding = 5.0; // 额外的区域用于悬停检测

        QRectF newRect;
        newRect.setRect(localViewRect.left(),
                        -penWidth / 2 - padding,
                        localViewRect.width(),
                        penWidth + 2 * padding);

        m_cachedBoundingRect = newRect;
    }

    return m_cachedBoundingRect;
}
void InfiniteLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(m_currentPen);
    QRectF rect = boundingRect();

    painter->drawLine(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
}

void InfiniteLineItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if (m_movable) {
        m_currentPen = m_hoverPen;
        update();
    }
    QGraphicsObject::hoverEnterEvent(event);
}

void InfiniteLineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (m_movable) {
        m_currentPen = m_pen;
        update();
    }
    QGraphicsObject::hoverLeaveEvent(event);
}

void InfiniteLineItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_movable && event->button() == Qt::LeftButton) {
        m_isMoving = true;
        m_currentPen = m_hoverPen; // 拖动时保持高亮
        update();
        event->accept();
    } else {
        event->ignore();
    }
}

void InfiniteLineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_movable && m_isMoving) {
        // 将鼠标当前位置(场景坐标)转换到父坐标系(通常也是场景坐标)
        QPointF newPos = mapToParent(mapFromScene(event->scenePos()));
        qreal newValue = 0;

        if (m_angle == 90) {
            newValue = newPos.x();
        } else {
            newValue = newPos.y();
        }
        setValue(newValue);

        emit positionChanged(newValue);
        event->accept();
    } else {
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void InfiniteLineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_movable && m_isMoving && event->button() == Qt::LeftButton) {
        m_isMoving = false;
        if (!isUnderMouse()) {
            m_currentPen = m_pen; // 恢复普通画笔
        }
        update();
        emit positionChangeFinished(value());
        event->accept();
    } else {
        QGraphicsObject::mouseReleaseEvent(event);
    }
}
