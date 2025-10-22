#ifndef ANNOTATIONPOINTITEM_H
#define ANNOTATIONPOINTITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>  // 用于动态计算文本大小

class AnnotationPointItem : public QGraphicsItem
{
public:
    AnnotationPointItem(int x, int y, int value, qreal baseScale, int imageWidth, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateFontSize(qreal currentScale);  // 更新字体大小（响应 scale 变化）

private:
    int m_x, m_y, m_value;
    qreal m_currentScale;  // 当前视图缩放比例
    qreal m_baseFontSize;  // 基础字体大小（基于图像大小计算）
    static const int BASE_POINT_RADIUS = 3;  // 基础点半径（动态调整）

    QString getInfoText() const;  // 获取格式化文本
    QRectF calculateTextRect() const;  // 动态计算文本 rect
};

#endif // ANNOTATIONPOINTITEM_H
