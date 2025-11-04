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
    return true;
}

bool DraggingHandleState::handleMouseMoveEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        QPointF scenePos = viewer->mapToScene(event->pos());
        machine()->updateDraggingHandle(scenePos);
        return true;
    }
    return false;
}

bool DraggingHandleState::handleMouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "DraggingHandleState: Mouse released, finishing drag and returning to Idle state.";
    machine()->finishDraggingHandle();
    machine()->setState(DrawingStateMachine::Idle);
    return true;
}

bool DraggingHandleState::handleWheelEvent(QWheelEvent *event)
{
    return true;
}
