#include "annotationpointitem.h"
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QDebug>

AnnotationPointItem::AnnotationPointItem(int x, int y, int value, qreal baseScale, int imageWidth, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_x(x), m_y(y), m_value(value), m_currentScale(baseScale), m_baseFontSize(0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setZValue(1001);

    // 优化：增大基础字体大小公式（基于图像宽度，更大初始值）
    m_baseFontSize = qMin(20.0 + (static_cast<qreal>(imageWidth) / 600.0), 30.0);
    qDebug() << "PointItem created: baseFontSize =" << m_baseFontSize << "for imageWidth" << imageWidth;
}

QRectF AnnotationPointItem::boundingRect() const
{
    // 动态计算：点 + 文本（优化后边界更宽松）
    qreal pointRadius = qMax(3.0, BASE_POINT_RADIUS * m_currentScale * 1.5);  // 优化：增大半径，最小3px
    QRectF textRect = calculateTextRect();
    qreal left = qMin(0.0, textRect.left() - pointRadius);
    qreal top = qMin(m_y - pointRadius * 2 - textRect.height(), m_y - pointRadius);  // 文本在上方
    qreal rightPadding = qMax(2.0, BASE_POINT_RADIUS * m_currentScale * 0.5);  // 动态
    qreal right = qMax(m_x + pointRadius, textRect.right() + rightPadding);
    qreal bottom = qMax(m_y + pointRadius, textRect.bottom() + pointRadius);

    return QRectF(left, top, right - left, bottom - top).adjusted(-3, -3, 3, 3);  // 优化：更大 padding
}
void AnnotationPointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 优化：动态半径和字体（更大）
    qreal pointRadius = qMax(4.0, BASE_POINT_RADIUS * m_currentScale * 2.0);
    qreal fontSize = m_baseFontSize * m_currentScale;
    if (fontSize < 12.0) fontSize = 12.0;
    if (fontSize > 24.0) fontSize = 24.0;  // 上限不变

    // 绘制点（红色小圆，边框和填充随scale变粗/大）
    painter->setPen(QPen(Qt::red, 3.0 * m_currentScale));
    painter->setBrush(Qt::red);
    painter->drawEllipse(QPointF(m_x, m_y), pointRadius, pointRadius);

    // 准备绘制文本
    QString text = getInfoText();
    QFont font("Arial", static_cast<int>(fontSize));
    painter->setFont(font);
    QRectF textRect = calculateTextRect();  // 动态 rect

    // 移除背景绘制，直接绘制红色文本
    painter->setPen(Qt::red);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);


    // 绘制 boundingRect 以可视化边界（调试用） ---
    // 设置青色边框（不填充），宽度1px
    painter->setPen(QPen(Qt::cyan, 1.0));
    painter->setBrush(Qt::NoBrush);  // 无填充
    painter->drawRect(boundingRect());  // 绘制边界矩形
}

void AnnotationPointItem::updateFontSize(qreal currentScale)
{
    if (!qFuzzyCompare(m_currentScale, currentScale)) {
        m_currentScale = currentScale;
        update();  // 重绘
        prepareGeometryChange();  // 更新 boundingRect
    }
}

QString AnnotationPointItem::getInfoText() const
{
    if (m_value != -1) {
        return QString("X:%1 Y:%2 value:%3").arg(m_x).arg(m_y).arg(m_value);
    } else {
        return QString("X:%1 Y:%2 value:N/A").arg(m_x).arg(m_y);
    }
}

QRectF AnnotationPointItem::calculateTextRect() const
{
    qreal fontSize = m_baseFontSize * m_currentScale;
    if (fontSize < 12.0) fontSize = 12.0;  // 保持最小
    QFont font("Arial", static_cast<int>(fontSize));
    // font.setBold(true);
    QFontMetrics fm(font);
    QString text = getInfoText();
    // 修正：移除此处多余的内边距计算，只获取文本的实际宽度和高度
    int textWidth = fm.horizontalAdvance(text);
    int textHeight = fm.height();

    // 位置：点右侧上方，动态间距
    qreal pointRadius = qMax(4.0, BASE_POINT_RADIUS * m_currentScale * 2.0);  // 保持上个优化
    qreal textX = m_x + pointRadius + qMax(6.0, fontSize * 0.3);  // 优化：间距动态，高scale下不拉太远
    qreal textY = m_y - textHeight / 2 - pointRadius;  // 上方居中点（不变）

    return QRectF(textX, textY, textWidth, textHeight);
}
