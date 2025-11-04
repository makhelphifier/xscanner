#include "drawingstatemachine.h"
#include "drawingstate.h"
#include "idlestate.h"
#include "panningstate.h"
#include "dragginghandlestate.h"
#include "gui/views/imageviewer.h"
#include "gui/items/rectroi.h"
#include "gui/items/handle.h"
#include "gui/items/roi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include "angledrawingstate.h"

DrawingStateMachine::DrawingStateMachine(ImageViewer* viewer, QObject *parent)
    : QObject(parent),
    m_viewer(viewer),
    m_currentStatePtr(nullptr),
   m_currentStateIsTemporary(false),
    m_currentlyDraggingHandle(nullptr),
    m_targetRoi(nullptr)
{
    // 创建所有状态实例
    m_idleState = new IdleState(this, this);
    m_panningState = new PanningState(this, this);
    m_draggingHandleState = new DraggingHandleState(this, this);
    m_angleDrawingState = new AngleDrawingState(this, this);

    // 设置初始状态
    setState(Idle);

    if (!m_viewer) {
        qWarning() << "DrawingStateMachine created with null ImageViewer!";
    }
}

DrawingStateMachine::~DrawingStateMachine()
{
    if (m_currentStatePtr && m_currentStateIsTemporary) {
        delete m_currentStatePtr;
        m_currentStatePtr = nullptr;
    }
}

void DrawingStateMachine::setState(StateType type)
{
    DrawingState* nextState = nullptr;
    switch (type) {
    case Idle:           nextState = m_idleState; break;
    case Panning:        nextState = m_panningState; break;
    case AngleDrawing:   nextState = m_angleDrawingState; break;
    case DraggingHandle: nextState = m_draggingHandleState; break;
    case Drawing:
    default:
        qWarning() << "Attempted to transition to an invalid or temporary state type:" << type;
        return;
    }

    // 调用新的重载，标记为 false (非临时)
    setState(nextState, false);
}
void DrawingStateMachine::setState(DrawingState* nextState, bool temporary)
{
    if (m_currentStatePtr == nextState || nextState == nullptr) {
        if (!nextState) {
            qWarning() << "Attempted to transition to a null state.";
        }
        return; // 没有变化或状态无效
    }

    if (m_currentStatePtr) {
        m_currentStatePtr->exit();
        if (m_currentStateIsTemporary) {
            qDebug() << "Deleting temporary state.";
            delete m_currentStatePtr;
        }
    }

    m_currentStatePtr = nextState;
    m_currentStateIsTemporary = temporary; // 记录新状态是否为临时
    m_currentStatePtr->enter(); // 调用新状态的进入方法

    qDebug() << "State transitioned to:" << m_currentStatePtr->metaObject()->className()
             << "(Temporary:" << temporary << ")";
}
DrawingStateMachine::StateType DrawingStateMachine::currentState() const
{
    if (m_currentStatePtr == m_idleState) return Idle;
    if (m_currentStatePtr == m_panningState) return Panning;
    if (m_currentStatePtr == m_draggingHandleState) return DraggingHandle;
    if (m_currentStatePtr == m_angleDrawingState) return AngleDrawing;
    return Idle; // 默认或错误情况
}

ImageViewer *DrawingStateMachine::viewer() const
{
 return m_viewer;
}
AngleDrawingState* DrawingStateMachine::angleDrawingState() const
{
    return m_angleDrawingState;
}
// --- 事件转发 ---

bool DrawingStateMachine::handleMousePressEvent(QMouseEvent *event)
{
    if (!m_currentStatePtr) return false;
    return m_currentStatePtr->handleMousePressEvent(event);
}

bool DrawingStateMachine::handleMouseMoveEvent(QMouseEvent *event)
{
    if (!m_currentStatePtr) return false;
    return m_currentStatePtr->handleMouseMoveEvent(event);
}

bool DrawingStateMachine::handleMouseReleaseEvent(QMouseEvent *event)
{
    if (!m_currentStatePtr) return false;
    return m_currentStatePtr->handleMouseReleaseEvent(event);
}

bool DrawingStateMachine::handleWheelEvent(QWheelEvent *event)
{
    if (!m_currentStatePtr) return false;
    return m_currentStatePtr->handleWheelEvent(event);
}



void DrawingStateMachine::startDraggingHandle(Handle* handle, const QPointF& scenePos)
{
    m_currentlyDraggingHandle = handle;
    if (!m_currentlyDraggingHandle || m_currentlyDraggingHandle->rois().isEmpty()) {
        qWarning() << "StateMachine: Attempting to drag invalid handle or handle with no ROIs.";
        m_currentlyDraggingHandle = nullptr;
        setState(Idle); // 回到 Idle
        return;
    }
    m_targetRoi = m_currentlyDraggingHandle->rois().first();
    if (m_targetRoi) {
        m_targetRoi->handleDragStarted(m_currentlyDraggingHandle);
        qDebug() << "StateMachine: Started dragging handle.";
    } else {
        qWarning() << "StateMachine: Handle is not connected to a valid ROI.";
        m_currentlyDraggingHandle = nullptr;
        setState(Idle); // 回到 Idle
    }
}

void DrawingStateMachine::updateDraggingHandle(const QPointF& scenePos)
{
    if (!m_currentlyDraggingHandle || !m_targetRoi) return;
    // 调用 ROI 的方法来处理句柄移动
    m_targetRoi->movePoint(m_currentlyDraggingHandle, scenePos, false);
}

void DrawingStateMachine::finishDraggingHandle()
{
    if (!m_currentlyDraggingHandle || !m_targetRoi) return;
    qDebug() << "StateMachine: Finished dragging handle.";
    QPointF finalScenePos = m_currentlyDraggingHandle->scenePos();
    m_targetRoi->movePoint(m_currentlyDraggingHandle, finalScenePos, true);
    m_targetRoi->handleDragFinished(m_currentlyDraggingHandle);
    m_currentlyDraggingHandle = nullptr;
    m_targetRoi = nullptr;
}
