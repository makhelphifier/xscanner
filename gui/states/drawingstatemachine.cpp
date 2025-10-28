// gui/states/drawingstatemachine.cpp
#include "drawingstatemachine.h"
#include "drawingstate.h" // 基类
#include "idlestate.h"
#include "panningstate.h"
#include "drawingrectstate.h"
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
    m_currentlyDrawingRoi(nullptr),
    m_currentlyDraggingHandle(nullptr),
    m_targetRoi(nullptr)
{
    // 创建所有状态实例
    m_idleState = new IdleState(this, this);
    m_panningState = new PanningState(this, this);
    m_drawingRectState = new DrawingRectState(this, this);
    m_draggingHandleState = new DraggingHandleState(this, this);

    // 设置初始状态
    setState(Idle);

    if (!m_viewer) {
        qWarning() << "DrawingStateMachine created with null ImageViewer!";
    }
}

DrawingStateMachine::~DrawingStateMachine()
{
    // QObject 会自动管理子对象，状态实例会被删除
}

void DrawingStateMachine::setState(StateType type)
{
    DrawingState* nextState = nullptr;
    switch (type) {
    case Idle:           nextState = m_idleState; break;
    case Panning:        nextState = m_panningState; break;
    case DrawingRect:    nextState = m_drawingRectState; break;
    case DraggingHandle: nextState = m_draggingHandleState; break;
    }

    if (m_currentStatePtr != nextState && nextState != nullptr) {
        StateType oldType = currentState(); // 获取旧类型
        if (m_currentStatePtr) {
            m_currentStatePtr->exit(); // 调用旧状态的退出方法
        }
        m_currentStatePtr = nextState;
        m_currentStatePtr->enter(); // 调用新状态的进入方法
        qDebug() << "State transitioned to:" << type;
        emit stateChanged(type); // 发送信号
    } else if (!nextState) {
        qWarning() << "Attempted to transition to an invalid state type:" << type;
    }
}

DrawingStateMachine::StateType DrawingStateMachine::currentState() const
{
    if (m_currentStatePtr == m_idleState) return Idle;
    if (m_currentStatePtr == m_panningState) return Panning;
    if (m_currentStatePtr == m_drawingRectState) return DrawingRect;
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


// --- 辅助方法实现 (与之前类似，供状态类调用) ---

void DrawingStateMachine::startDrawingRect(const QPointF& scenePos)
{
    if (!m_viewer) return;
    m_currentlyDrawingRoi = new RectROI(scenePos, QSizeF(0, 0)); // 使用 RectROI
    m_viewer->scene()->addItem(m_currentlyDrawingRoi);
    qDebug() << "StateMachine: Started drawing RectROI at:" << scenePos;
}

void DrawingStateMachine::updateDrawingRect(const QPointF& scenePos)
{
    if (!m_currentlyDrawingRoi || !m_viewer) return;
    QRectF rect = QRectF(m_startDragPos, scenePos).normalized();
    m_currentlyDrawingRoi->setPos(rect.topLeft());
    m_currentlyDrawingRoi->setSize(rect.size());
}

void DrawingStateMachine::finishDrawingRect()
{
    if (!m_currentlyDrawingRoi || !m_viewer) return;
    qDebug() << "StateMachine: Finished drawing RectROI:" << m_currentlyDrawingRoi->boundingRect();
    if (m_currentlyDrawingRoi->size().width() < 5 || m_currentlyDrawingRoi->size().height() < 5) { // 增加最小尺寸判断
        m_viewer->scene()->removeItem(m_currentlyDrawingRoi);
        delete m_currentlyDrawingRoi;
        qDebug() << "StateMachine: Removed ROI because it was too small.";
    }
    m_currentlyDrawingRoi = nullptr;
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
