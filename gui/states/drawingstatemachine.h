// gui/states/drawingstatemachine.h
#ifndef DRAWINGSTATEMACHINE_H
#define DRAWINGSTATEMACHINE_H

#include <QObject>
#include <QPointer>
#include <QPointF>

// 前向声明
class ImageViewer;
class DrawingState; // 基类
class IdleState;
class PanningState;
class DrawingRectState;
class DraggingHandleState;
class QMouseEvent;
class QWheelEvent;
class Handle;
class ROI;
class RectROI;

class DrawingStateMachine : public QObject
{
    Q_OBJECT

public:
    // 可选：保留枚举用于信号或外部查询
    enum StateType {
        Idle,
        Panning,
        DrawingRect,
        DraggingHandle
        // DraggingROI // 如果需要
    };
    Q_ENUM(StateType)

    explicit DrawingStateMachine(ImageViewer* viewer, QObject *parent = nullptr);
    ~DrawingStateMachine();

    // 事件处理入口 (转发给当前状态)
    bool handleMousePressEvent(QMouseEvent *event);
    bool handleMouseMoveEvent(QMouseEvent *event);
    bool handleMouseReleaseEvent(QMouseEvent *event);
    bool handleWheelEvent(QWheelEvent *event);

    // 状态转换
    void setState(StateType type);
    StateType currentState() const; // 返回当前状态类型

    // 访问器和辅助方法 (供状态类调用)
    ImageViewer* viewer() const ;
    void setLastMousePos(const QPoint& pos) { m_lastMousePos = pos; }
    QPoint lastMousePos() const { return m_lastMousePos; }
    void setStartDragPos(const QPointF& pos) { m_startDragPos = pos; }
    QPointF startDragPos() const { return m_startDragPos; }

    void startDrawingRect(const QPointF& scenePos);
    void updateDrawingRect(const QPointF& scenePos);
    void finishDrawingRect();

    void startDraggingHandle(Handle* handle, const QPointF& scenePos);
    void updateDraggingHandle(const QPointF& scenePos);
    void finishDraggingHandle();

signals:
    void stateChanged(StateType newState); // 信号可以发送枚举类型

private:
    QPointer<ImageViewer> m_viewer;
    DrawingState* m_currentStatePtr; // 当前状态对象指针

    // 持有所有状态的实例
    IdleState* m_idleState;
    PanningState* m_panningState;
    DrawingRectState* m_drawingRectState;
    DraggingHandleState* m_draggingHandleState;

    // 状态相关临时数据 (由状态类通过状态机接口访问/修改)
    QPoint m_lastMousePos;        // 视图坐标，用于平移计算
    QPointF m_startDragPos;       // 场景坐标，用于绘制/拖动计算
    RectROI* m_currentlyDrawingRoi; // 正在绘制的ROI
    Handle* m_currentlyDraggingHandle; // 正在拖动的Handle
    ROI* m_targetRoi;             // 拖动Handle时关联的ROI
};

#endif // DRAWINGSTATEMACHINE_H
