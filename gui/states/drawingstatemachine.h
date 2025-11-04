#ifndef DRAWINGSTATEMACHINE_H
#define DRAWINGSTATEMACHINE_H

#include <QObject>
#include <QPointer>
#include <QPointF>

class ImageViewer;
class DrawingState;
class IdleState;
class PanningState;
class DrawingRectState;
class DraggingHandleState;
class QMouseEvent;
class QWheelEvent;
class Handle;
class ROI;
class RectROI;
class AngleDrawingState;

class DrawingStateMachine : public QObject
{
    Q_OBJECT

public:
    enum StateType {
        Idle,
        Panning,
        Drawing,
        DraggingHandle,
        AngleDrawing,

    };
    Q_ENUM(StateType)

    explicit DrawingStateMachine(ImageViewer* viewer, QObject *parent = nullptr);
    ~DrawingStateMachine();

    // 事件处理入口 (转发给当前状态)
    bool handleMousePressEvent(QMouseEvent *event);
    bool handleMouseMoveEvent(QMouseEvent *event);
    bool handleMouseReleaseEvent(QMouseEvent *event);
    bool handleWheelEvent(QWheelEvent *event);

    void setState(DrawingState* newState, bool temporary = false);
    // 状态转换
    void setState(StateType type);
    StateType currentState() const; // 返回当前状态类型

    // 访问器和辅助方法 (供状态类调用)
    ImageViewer* viewer() const ;
    void setLastMousePos(const QPoint& pos) { m_lastMousePos = pos; }
    QPoint lastMousePos() const { return m_lastMousePos; }
    void setStartDragPos(const QPointF& pos) { m_startDragPos = pos; }
    QPointF startDragPos() const { return m_startDragPos; }
    AngleDrawingState* angleDrawingState() const;


    void startDraggingHandle(Handle* handle, const QPointF& scenePos);
    void updateDraggingHandle(const QPointF& scenePos);
    void finishDraggingHandle();

signals:
    void stateChanged(StateType newState);

private:
    QPointer<ImageViewer> m_viewer;
    DrawingState* m_currentStatePtr; // 当前状态对象指针
    bool m_currentStateIsTemporary = false;
    // 持有所有状态的实例
    IdleState* m_idleState;
    PanningState* m_panningState;
    DraggingHandleState* m_draggingHandleState;

    // 状态相关临时数据 (由状态类通过状态机接口访问/修改)
    QPoint m_lastMousePos;
    QPointF m_startDragPos;
    Handle* m_currentlyDraggingHandle;
    ROI* m_targetRoi;
    AngleDrawingState* m_angleDrawingState;
};

#endif // DRAWINGSTATEMACHINE_H
