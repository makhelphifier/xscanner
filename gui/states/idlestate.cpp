// gui/states/idlestate.cpp
#include "idlestate.h"
#include "drawingstatemachine.h"
#include "gui/views/imageviewer.h"
#include "gui/items/handle.h"
#include "gui/items/roi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsItem>
#include <QDebug>

IdleState::IdleState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent) {}

bool IdleState::handleMousePressEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer) return false;

    QPointF scenePos = viewer->mapToScene(event->pos());
    machine()->setLastMousePos(event->pos()); // 记录视图坐标用于可能的平移
    machine()->setStartDragPos(scenePos);     // 记录场景坐标用于可能的绘制/拖动

    QGraphicsItem* item = viewer->itemAt(event->pos());
    Handle* handle = qgraphicsitem_cast<Handle*>(item);
    ROI* roi = nullptr;
    QGraphicsItem* currentItem = item;
    while (currentItem && !roi) {
        roi = qgraphicsitem_cast<ROI*>(currentItem);
        currentItem = currentItem->parentItem();
    }

    if (handle && event->button() == Qt::LeftButton) {
        machine()->startDraggingHandle(handle, scenePos); // 通知状态机开始拖动句柄的数据记录
        machine()->setState(DrawingStateMachine::DraggingHandle); // 切换状态
        return true; // 事件已处理
    } else if (roi && event->button() == Qt::LeftButton && item == roi) { // 点击ROI本体
        qDebug() << "IdleState: Clicked on ROI Body, letting ROI handle drag.";
        // ROI 自己处理拖动，状态机不介入，事件向下传递
        return false;
    } else if (event->button() == Qt::LeftButton) {
        // 点击背景 -> 准备平移
        machine()->setState(DrawingStateMachine::Panning); // 切换状态
        qDebug() << "IdleState: Left click on background, entering Panning state.";
        // 返回 false，让 QGraphicsView 的默认或自定义平移逻辑启动
        return false;
    } else if (event->button() == Qt::RightButton && viewer->isDrawingEnabled()) {
        // 右键点击背景 -> 开始绘制矩形
        machine()->startDrawingRect(scenePos); // 通知状态机创建ROI
        machine()->setState(DrawingStateMachine::DrawingRect); // 切换状态
        qDebug() << "IdleState: Right click, entering DrawingRect state.";
        return true; // 事件已处理
    }

    return false; // 其他情况不处理
}

bool IdleState::handleMouseMoveEvent(QMouseEvent *event)
{
    // Idle 状态下，鼠标移动通常只用于更新像素信息，由 ImageViewer 处理
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        viewer->updatePixelInfo(viewer->mapToScene(event->pos()));
    }
    return false; // 不消耗事件
}

bool IdleState::handleMouseReleaseEvent(QMouseEvent *event)
{
    // 在 Idle 状态下释放鼠标，通常不需要做特别处理
    return false;
}

bool IdleState::handleWheelEvent(QWheelEvent *event)
{
    // 在 Idle 状态下，滚轮用于缩放视图
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        qreal scaleFactor = qPow(1.15, event->angleDelta().y() / 120.0);
        viewer->scaleView(scaleFactor);
        return true; // 事件已处理
    }
    return false;
}
