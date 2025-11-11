// states/panningstate.cpp
#include "panningstate.h"
#include "drawingstatemachine.h"
#include "views/imageviewer.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include "logger.h"

PanningState::PanningState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent) {}

bool PanningState::handleMousePressEvent(QMouseEvent *event)
{
    // 在平移状态下再次按下鼠标通常不做任何事
    return true; // 消耗事件
}

bool PanningState::handleMouseMoveEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        QPoint delta = event->pos() - machine()->lastMousePos();
        machine()->setLastMousePos(event->pos());
        viewer->translateView(delta); // 调用视图平移
        return true; // 事件已处理
    }
    return false;
}

bool PanningState::handleMouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "PanningState: Mouse released, returning to Idle state.";
    machine()->setState(DrawingStateMachine::Idle); // 返回 Idle 状态
    ImageViewer* viewer = machine()->viewer();
    if(viewer) viewer->panFinished(); // 通知视图平移结束
    // 返回 false 可能允许 QGraphicsView 处理一些释放逻辑
    return false;
}

bool PanningState::handleWheelEvent(QWheelEvent *event)
{
    // 平移过程中通常忽略滚轮事件
    qDebug() << "PanningState: Ignoring wheel event.";
    return true; // 消耗事件，防止意外缩放
}
