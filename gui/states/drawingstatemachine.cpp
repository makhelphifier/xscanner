// gui/states/drawingstatemachine.cpp
#include "drawingstatemachine.h"
#include "drawingstate.h" // 基类
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
    // QObject 会自动管理子对象，状态实例会被删除
}

void DrawingStateMachine::setState(StateType type)
{
    DrawingState* nextState = nullptr;
    switch (type) {
    case Idle:           nextState = m_idleState; break;
    case Panning:        nextState = m_panningState; break;
    // case DrawingRect:    // <-- 移除
    case DraggingHandle: nextState = m_draggingHandleState; break;
    case Drawing:        // 'Drawing' 状态是临时的，不能用这个函数设置
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

    // 退出旧状态
    if (m_currentStatePtr) {
        m_currentStatePtr->exit();
        // 如果旧状态是临时的，立即删除
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

    // emit stateChanged(currentState()); // ++ 更新信号发射 ++
}
DrawingStateMachine::StateType DrawingStateMachine::currentState() const
{
    if (m_currentStatePtr == m_idleState) return Idle;
    if (m_currentStatePtr == m_panningState) return Panning;
    if (m_currentStatePtr == m_draggingHandleState) return DraggingHandle;
    return Idle; // 默认或错误情况
}

ImageViewer *DrawingStateMachine::viewer() const
{
 return m_viewer;
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
    // 获取 Handle 当前的最终位置来调用 movePoint(true)
    QPointF finalScenePos = m_currentlyDraggingHandle->scenePos();
    m_targetRoi->movePoint(m_currentlyDraggingHandle, finalScenePos, true);
    m_targetRoi->handleDragFinished(m_currentlyDraggingHandle);

    m_currentlyDraggingHandle = nullptr;
    m_targetRoi = nullptr;
}
