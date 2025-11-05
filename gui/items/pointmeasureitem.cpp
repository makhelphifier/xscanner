#include "pointmeasureitem.h"
#include "gui/views/imageviewer.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QFont>
#include <QDebug>
#include <QVariant>
#include <QGraphicsScene>
#include "gui/viewmodels/imageviewmodel.h"

PointMeasureItem::PointMeasureItem(const QPointF& pos, ImageViewModel* viewModel, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_viewModel(viewModel)
{
    if (!m_viewModel) {
        qWarning() << "PointMeasureItem created with null viewModel!";
    }

    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setZValue(1001);
    setPos(pos);

    m_textOffset = QPointF(MARKER_SIZE + 2, -MARKER_SIZE);

    // 在构造函数中立即计算初始文本
    // 这会设置 m_text
    updateTextAndPos(pos);
}

QRectF PointMeasureItem::boundingRect() const
{
    // 1. 标记的矩形 (在 0,0)
    QRectF crossRect = QRectF(-MARKER_SIZE, -MARKER_SIZE, MARKER_SIZE * 2, MARKER_SIZE * 2);

    // 2. 动态计算文本矩形
    QRectF textRect = calculateTextRect();

    // 3. 合并
    QRectF unitedRect = crossRect.united(textRect);

    qDebug() << "[PointMeasureItem] boundingRect() called. TextRect:" << textRect << "Final unitedRect:" << unitedRect.adjusted(-2, -2, 2, 2);

    return unitedRect.adjusted(-2, -2, 2, 2); // 额外 padding
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

    painter->setFont(QFont("Arial", 10));
    painter->drawText(calculateTextRect(), 0, m_text);
}

void PointMeasureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // 首先，让基类处理移动
    QGraphicsObject::mouseReleaseEvent(event);

    // 检查是否真的移动了
    if (event->button() == Qt::LeftButton && m_viewModel) {
        updateTextAndPos(pos());
    }
}
/**
 * @brief 2. updateTextAndPos 函数
 *
 * 在这里添加动态翻转 m_textOffset 的逻辑
 */
void PointMeasureItem::updateTextAndPos(const QPointF& scenePos)
{
    int x = qRound(scenePos.x());
    int y = qRound(scenePos.y());
    int value = -1;

    if (m_viewModel && m_viewModel->imageBounds().contains(scenePos)) {
        value = m_viewModel->getPixelValue(x, y);
    }
    // 1. 更新文本
    if (value != -1) {
        m_text = QString("X: %1\nY: %2\nVal: %3").arg(x).arg(y).arg(value);
    } else {
        m_text = QString("X: %1\nY: %2\nVal: N/A").arg(x).arg(y);
    }

    //  动态计算 m_textOffset 以避免超出图像边界
    if (m_viewModel) {
        // 增加这个值，文本就会在距离边界更远的地方翻转
        const qreal FLIP_BUFFER = 500.0;

        // 1. 获取文本的预期大小
        QFontMetrics fm(QFont("Arial", 10));
        QRect textBlockRect = fm.boundingRect(QRect(0, 0, 500, 0), 0, m_text);
        qreal textWidth = textBlockRect.width();
        qreal textHeight = textBlockRect.height();

        // 2. 获取图像边界 (在场景坐标系中)
        QRectF bounds = m_viewModel->imageBounds();

        // 3. 定义文本到十字星的边距
        qreal h_padding = MARKER_SIZE + 2;
        qreal v_padding = MARKER_SIZE + 2;

        qreal finalOffsetX;
        qreal finalOffsetY;

        // 4. 决定水平 (X) 位置
        //    检查如果放右边，是否会超出右边界？
        if (scenePos.x() + h_padding + textWidth + FLIP_BUFFER > bounds.right()) {
            // 是: 翻转到左侧
            finalOffsetX = -h_padding - textWidth;
        } else {
            // 否: 默认放在右侧
            finalOffsetX = h_padding;
        }

        // 5. 决定垂直 (Y) 位置
        //    检查如果放上边，是否会超出上边界？
        if (scenePos.y() - v_padding - textHeight - FLIP_BUFFER < bounds.top()) {
            // 是: 翻转到下方
            finalOffsetY = v_padding;
        } else {
            // 否: 默认放在上方
            finalOffsetY = -v_padding - textHeight;
        }

        // 6. 设置动态计算出的偏移量
        m_textOffset = QPointF(finalOffsetX, finalOffsetY);
    }

    prepareGeometryChange();
    update();
}
/**
 * @brief 动态计算文本的边界框（在局部坐标系中）
 *
 * 在 boundingRect() 中被调用，以确保边界始终是正确的
 */
QRectF PointMeasureItem::calculateTextRect() const
{
    QFontMetrics fm(QFont("Arial", 10));
    // m_text 可能是空的（例如在构造的瞬间），但 updateTextAndPos 会立即设置它
    // 我们给一个 QRect()，flags 设为 0（因为 m_text 包含 \n）
    QRect rectAtOrigin = fm.boundingRect(QRect(0, 0, 500, 0), 0, m_text);

    // 将这个矩形平移到我们的 m_textOffset 位置
    return rectAtOrigin.translated(m_textOffset.toPoint());
}
