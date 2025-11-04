#include "drawingstate.h"
#include "drawingstatemachine.h"
DrawingState::DrawingState(DrawingStateMachine* machine, QObject *parent)
    : QObject(parent), m_machine(machine)
{
}

