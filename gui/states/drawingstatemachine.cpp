#include "drawingstatemachine.h"
#include "drawingstate.h"

DrawingStateMachine::DrawingStateMachine(ImageViewer* viewer, QObject *parent)
    : QObject(parent), m_viewer(viewer), m_currentState(nullptr)
{
}

DrawingStateMachine::~DrawingStateMachine() = default;

void DrawingStateMachine::setState(DrawingState* state)
{
    m_currentState.reset(state);
}

void DrawingStateMachine::handleMousePress(QMouseEvent *event)
{
    if (m_currentState) {
        m_currentState->mousePressEvent(event);
    }
}

void DrawingStateMachine::handleMouseMove(QMouseEvent *event)
{
    if (m_currentState) {
        m_currentState->mouseMoveEvent(event);
    }
}

void DrawingStateMachine::handleMouseRelease(QMouseEvent *event)
{
    if (m_currentState) {
        m_currentState->mouseReleaseEvent(event);
    }
}
