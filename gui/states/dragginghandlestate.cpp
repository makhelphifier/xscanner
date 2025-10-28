// gui/states/dragginghandlestate.cpp
#include "dragginghandlestate.h"
#include "drawingstatemachine.h"
#include "gui/views/imageviewer.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>


DraggingHandleState::DraggingHandleState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent) {}

bool DraggingHandleState::handleMousePressEvent(QMouseEvent *event)
{
    // 在拖动状态下再次按下鼠标通常不做任何事
    return true;
}

bool DraggingHandleState::handleMouseMoveEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        QPointF scenePos = viewer->mapToScene(event->pos());
        machine()->updateDraggingHandle(scenePos); // 通知状态机更新 Handle 位置
        return true; // 事件已处理
    }
    return false;
}

bool DraggingHandleState::handleMouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "DraggingHandleState: Mouse released, finishing drag and returning to Idle state.";
    machine()->finishDraggingHandle(); // 完成拖动
    machine()->setState(DrawingStateMachine::Idle); // 返回 Idle 状态
    return true; // 事件已处理
}

bool DraggingHandleState::handleWheelEvent(QWheelEvent *event)
{
    // 拖动过程中通常忽略滚轮事件
    qDebug() << "DraggingHandleState: Ignoring wheel event.";
    return true;
}
