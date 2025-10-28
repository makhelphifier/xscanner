// gui/states/drawingrectstate.h
#ifndef DRAWINGRECTSTATE_H
#define DRAWINGRECTSTATE_H

#include "drawingstate.h"

class DrawingRectState : public DrawingState
{
    Q_OBJECT
public:
    explicit DrawingRectState(DrawingStateMachine* machine, QObject *parent = nullptr);

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;
};

#endif // DRAWINGRECTSTATE_H
