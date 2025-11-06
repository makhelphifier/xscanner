#ifndef IDLESTATE_H
#define IDLESTATE_H

#include "drawingstate.h"

class IdleState : public DrawingState
{
    Q_OBJECT
public:
    explicit IdleState(DrawingStateMachine* machine, QObject *parent = nullptr);

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;
};

#endif // IDLESTATE_H
