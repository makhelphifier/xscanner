#include "pointmeasureitem.h"
#include "gui/views/imageviewer.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QFont>
#include <QDebug>
#include <QVariant>
#include <QGraphicsScene>

PointMeasureItem::PointMeasureItem(const QPointF& pos, ImageViewer* viewer, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_viewer(viewer)
{
    if (!m_viewer) {
        qWarning() << "PointMeasureItem created with null viewer!";
    }

    // 关键：设置此标志
    // 1. paint() 将在像素坐标中绘制
    // 2. 项目本身不会随视图缩放
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    setZValue(1001); // 确保在图像上

    // 设置 item 的场景位置
    setPos(pos);

    m_textOffset = QPointF(MARKER_SIZE + 2, -MARKER_SIZE);

    // 设置初始文本
    // updateTextAndPos(pos);


}

QRectF PointMeasureItem::boundingRect() const
{
    // 返回一个包含十字标记和文本的矩形
    QRectF rect = QRectF(-MARKER_SIZE, -MARKER_SIZE, MARKER_SIZE * 2, MARKER_SIZE * 2);
    rect = rect.united(m_textRect); // m_textRect 是 updateTextAndPos 计算的
    return rect.adjusted(-2, -2, 2, 2); // 额外 padding
}

void PointMeasureItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(Qt::yellow, 1); // 始终为 1 像素宽
    pen.setCosmetic(true); // 确保 1 像素
    painter->setPen(pen);

    // 1. 绘制十字标记 (在本地 (0,0) )
    painter->drawLine(-MARKER_SIZE, 0, MARKER_SIZE, 0);
    painter->drawLine(0, -MARKER_SIZE, 0, MARKER_SIZE);

    // 2. 绘制文本
    painter->setFont(QFont("Arial", 10));
    painter->drawText(m_textOffset, m_text);

    // (可选) 绘制调试边界
    // painter->setPen(Qt::cyan);
    // painter->drawRect(boundingRect());
}

void PointMeasureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // 首先，让基类处理移动
    QGraphicsObject::mouseReleaseEvent(event);

    // 检查是否真的移动了
    if (event->button() == Qt::LeftButton && m_viewer) {
        // 更新文本为 item 的新场景位置
        updateTextAndPos(pos());
    }
}

void PointMeasureItem::updateTextAndPos(const QPointF& scenePos)
{
    int x = qRound(scenePos.x());
    int y = qRound(scenePos.y());
    int value = -1;

    if (m_viewer && m_viewer->imageBounds().contains(scenePos)) {
        value = m_viewer->getPixelValue(x, y);
    }

    // 更新文本
    if (value != -1) {
        m_text = QString("X: %1\n Y: %2\n Val: %3").arg(x).arg(y).arg(value);
    } else {
        m_text = QString("X: %1\n Y: %2\n Val: N/A").arg(x).arg(y);
    }


    // 更新文本包围盒
    QFontMetrics fm(QFont("Arial", 10));
    // 1. 先在 (0,0) 计算多行文本的边界矩形
    //    我们给一个 QRect()，flags 设为 0（因为 m_text 包含 \n）
    // QRect rectAtOrigin = fm.boundingRect(QRect(0,0,0,0), 0, m_text);
    QRect rectAtOrigin = fm.boundingRect(QRect(0, 0, 500, 0), 0, m_text);
    // 2. 然后将这个矩形平移到我们的 m_textOffset 位置
    m_textRect = rectAtOrigin.translated(m_textOffset.toPoint());
    // 通知 Qt 我们的总包围盒 (boundingRect) 已经改变
    prepareGeometryChange();

    // 请求重绘
    update();
}


QVariant PointMeasureItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // 当 item 第一次被添加到 scene 时，会触发这个
    if (change == ItemSceneHasChanged && scene()) {
        // 此时 item 已经 context，可以安全地计算几何
        updateTextAndPos(pos());
    }

    return QGraphicsObject::itemChange(change, value);
}
