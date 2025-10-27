// handle.cpp

#include "handle.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>

// 不同类型句柄的形状定义（边数，起始角度）
static const QMap<HandleType, QPair<int, qreal>> handle_shapes = {
    {HandleType::Scale,       {4, M_PI / 4.0}}, // 方形
    {HandleType::Rotate,      {12, 0}},         // 多边形（近似圆形）
    {HandleType::ScaleRotate, {12, 0}},
    // 可以添加更多类型
};

Handle::Handle(HandleType type, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_type(type)
{
    // !!! 关键：设置此标志可使句柄忽略场景的缩放和旋转变换，保持固定的屏幕像素大小
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    buildPath();
}

void Handle::connectROI(ROI* roi)
{
    if (roi && !m_rois.contains(roi)) {
        m_rois.append(roi);
    }
}

void Handle::disconnectROI(ROI* roi)
{
    m_rois.removeAll(roi);
}

void Handle::setPen(const QPen& pen)
{
    m_pen = pen;
    if (!m_isMoving && !isUnderMouse()) {
        m_currentPen = m_pen;
        update();
    }
}

void Handle::setHoverPen(const QPen& pen)
{
    m_hoverPen = pen;
    if (!m_isMoving && isUnderMouse()) {
        m_currentPen = m_hoverPen;
        update();
    }
}

QRectF Handle::boundingRect() const
{
    return m_path.boundingRect();
}

void Handle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(m_currentPen);
    painter->drawPath(m_path);
}

void Handle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_currentPen = m_hoverPen;
    update();
}

void Handle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_currentPen = m_pen;
    update();
}

void Handle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isMoving = true;
        event->accept();
        for (ROI* roi : qAsConst(m_rois)) {
            roi->regionChangeStarted(roi); // 通知ROI拖动开始
        }
    } else {
        event->ignore();
    }
}

void Handle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_isMoving) {
        event->ignore();
        return;
    }
    // 获取句柄在场景中的新位置
    QPointF newScenePos = event->scenePos();

    // 通知所有关联的ROI，它们需要根据句柄的新位置进行更新
    for (ROI* roi : qAsConst(m_rois)) {
        // 调用ROI的movePoint方法，让ROI处理这个移动
        // (movePoint将在下一步中实现)
        roi->movePoint(this, newScenePos, false);
    }
    event->accept();
}

void Handle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isMoving && event->button() == Qt::LeftButton) {
        m_isMoving = false;
        QPointF finalScenePos = event->scenePos();
        for (ROI* roi : qAsConst(m_rois)) {
            roi->movePoint(this, finalScenePos, true); // 最后一次更新，并标记为“完成”
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void Handle::buildPath()
{
    m_path = QPainterPath();
    auto shape_info = handle_shapes.value(m_type, {4, M_PI/4.0});
    int sides = shape_info.first;
    qreal startAngle = shape_info.second;

    qreal angle_increment = 2 * M_PI / sides;
    for (int i = 0; i < sides; ++i) {
        qreal angle = startAngle + i * angle_increment;
        QPointF p(m_radius * cos(angle), m_radius * sin(angle));
        if (i == 0)
            m_path.moveTo(p);
        else
            m_path.lineTo(p);
    }
    m_path.closeSubpath();
}





