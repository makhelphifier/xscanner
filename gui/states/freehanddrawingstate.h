#ifndef FREEHANDDRAWINGSTATE_H
#define FREEHANDDRAWINGSTATE_H

#include "drawingstate.h"
#include <QPainterPath>

class QGraphicsPathItem;

/**
 * @brief 专用于 "按下-拖动" 绘制自由轨迹的状态
 */
class FreehandDrawingState : public DrawingState
{
    Q_OBJECT
public:
    explicit FreehandDrawingState(DrawingStateMachine* machine, QObject *parent = nullptr);
    virtual ~FreehandDrawingState() override;

    void enter() override;
    void exit() override;

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;
    bool handleMouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void clearPreview();

    bool m_isDrawing = false;
    QPainterPath m_currentPath;
    QGraphicsPathItem* m_previewItem = nullptr;
};

#endif // FREEHANDDRAWINGSTATE_H
