#ifndef ANNOTATIONLINEITEM_H
#define ANNOTATIONLINEITEM_H

#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>

class AnnotationLineItem : public QGraphicsLineItem
{
public:
    // 构造函数1: 绘制完成的线段
    AnnotationLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = nullptr);

    // 构造函数2: 用于开始绘制的新线段（只给定起点）
    AnnotationLineItem(const QPointF &startPoint, QGraphicsItem *parent = nullptr);

protected:
    // 重写鼠标按下事件：用于捕获第二次点击（终点）
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    // 重写悬停移动事件：用于更新终点跟随光标移动
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool m_drawing = false; // 标记是否处于绘制过程中
    QPointF m_startPoint;   // 存储起点

    void setupPen();        // 统一设置画笔样式
};

#endif // ANNOTATIONLINEITEM_H
