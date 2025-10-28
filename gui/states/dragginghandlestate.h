// gui/states/dragginghandlestate.h
#ifndef DRAGGINGHANDLESTATE_H
#define DRAGGINGHANDLESTATE_H

#include "drawingstate.h"

class DraggingHandleState : public DrawingState
{
    Q_OBJECT
public:
    explicit DraggingHandleState(DrawingStateMachine* machine, QObject *parent = nullptr);

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;
};

#endif // DRAGGINGHANDLESTATE_H
