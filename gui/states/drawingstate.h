// gui/states/drawingstate.h
#ifndef DRAWINGSTATE_H
#define DRAWINGSTATE_H

#include <QObject> // 包含 QObject 以便使用 qobject_cast 等

// 前向声明
class DrawingStateMachine;
class QMouseEvent;
class QWheelEvent;

class DrawingState : public QObject // 继承 QObject 以便使用信号槽等 (可选)
{
    Q_OBJECT
public:
    explicit DrawingState(DrawingStateMachine* machine, QObject *parent = nullptr);
    virtual ~DrawingState() = default;

    // 进入/退出状态时可能执行的操作 (可选)
    virtual void enter() {}
    virtual void exit() {}

    // 事件处理接口 (返回 true 表示事件已被处理)
    virtual bool handleMousePressEvent(QMouseEvent *event) = 0;
    virtual bool handleMouseMoveEvent(QMouseEvent *event) = 0;
    virtual bool handleMouseReleaseEvent(QMouseEvent *event) = 0;
    virtual bool handleWheelEvent(QWheelEvent *event) = 0;
    /**
     * @brief [新增] 处理双击事件 (默认为 false)
     *
     * 因为它不是纯虚函数，现有的状态类不需要修改。
     */
    virtual bool handleMouseDoubleClickEvent(QMouseEvent *event) { Q_UNUSED(event); return false; }
protected:
    DrawingStateMachine* machine() const { return m_machine; } // 提供访问状态机的接口

private:
    DrawingStateMachine* m_machine; // 指向所属的状态机
};

#endif // DRAWINGSTATE_H
