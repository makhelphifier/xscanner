#include "annotationlineitem.h"
#include <QPen>
#include <QGraphicsSceneHoverEvent>
#include <QApplication> // 用于恢复光标
#include <QDebug>

// 统一设置画笔样式
void AnnotationLineItem::setupPen()
{
    // 1. 设置画笔样式，使其在医学图像上更显眼
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    setPen(pen);
}

// 构造函数1: 绘制完成的线段
AnnotationLineItem::AnnotationLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent)
    : QGraphicsLineItem(x1, y1, x2, y2, parent)
{
    setupPen();
    // 设置Item的标志，使其可被选中和拖动
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

// 构造函数2: 用于开始绘制的新线段
AnnotationLineItem::AnnotationLineItem(const QPointF &startPoint, QGraphicsItem *parent)
    : QGraphicsLineItem(QLineF(startPoint, startPoint), parent),
    m_drawing(true),
    m_startPoint(startPoint)
{
    setupPen();
    // 绘制过程中，不应被选中和移动
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    // 启用悬停事件，以便在不按下鼠标按钮时也能接收移动事件
    setAcceptHoverEvents(true);
}

void AnnotationLineItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug()<<"aaa";
    if (m_drawing) {
        // 更新线段终点以跟随鼠标
        QLineF newLine(m_startPoint, event->scenePos());
        setLine(newLine);
        event->accept();
        return;
    }
    QGraphicsLineItem::hoverMoveEvent(event);
}

void AnnotationLineItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_drawing && event->button() == Qt::LeftButton) {
        // 第二次点击：结束绘制
        m_drawing = false;
        QLineF finalLine(m_startPoint, event->scenePos());
        setLine(finalLine);

        // 恢复选中和移动功能
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);

        // 停止接收悬停事件
        setAcceptHoverEvents(false);



        event->accept();
        return;
    }

    QGraphicsLineItem::mousePressEvent(event);
}
