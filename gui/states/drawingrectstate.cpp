// gui/states/drawingrectstate.cpp
#include "drawingrectstate.h"
#include "drawingstatemachine.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include "gui/views/imageviewer.h"
DrawingRectState::DrawingRectState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent) {}

bool DrawingRectState::handleMousePressEvent(QMouseEvent *event)
{
    // 在绘制状态下再次按下鼠标通常不做任何事
    return true;
}

bool DrawingRectState::handleMouseMoveEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        QPointF scenePos = viewer->mapToScene(event->pos());
        machine()->updateDrawingRect(scenePos); // 更新状态机中正在绘制的ROI
        return true; // 事件已处理
    }
    return false;
}

bool DrawingRectState::handleMouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "DrawingRectState: Mouse released, finishing drawing and returning to Idle state.";
    machine()->finishDrawingRect(); // 完成绘制
    machine()->setState(DrawingStateMachine::Idle); // 返回 Idle 状态
    return true; // 事件已处理
}

bool DrawingRectState::handleWheelEvent(QWheelEvent *event)
{
    // 绘制过程中通常忽略滚轮事件
    qDebug() << "DrawingRectState: Ignoring wheel event.";
    return true;
}
