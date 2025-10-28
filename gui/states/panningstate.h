// gui/states/panningstate.h
#ifndef PANNINGSTATE_H
#define PANNINGSTATE_H

#include "drawingstate.h"

class PanningState : public DrawingState
{
    Q_OBJECT
public:
    explicit PanningState(DrawingStateMachine* machine, QObject *parent = nullptr);

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;
};

#endif // PANNINGSTATE_H
