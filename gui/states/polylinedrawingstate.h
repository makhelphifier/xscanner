#ifndef POLYLINEDRAWINGSTATE_H
#define POLYLINEDRAWINGSTATE_H

#include "drawingstate.h"
#include <QList>
#include <QPointF>
#include <QWheelEvent>

class QGraphicsPathItem;
class QGraphicsLineItem;

/**
 * @brief 专用于 "多点点击" 绘制多线段的状态
 */
class PolylineDrawingState : public DrawingState
{
    Q_OBJECT
public:
    explicit PolylineDrawingState(DrawingStateMachine* machine, QObject *parent = nullptr);
    virtual ~PolylineDrawingState() override;

    void enter() override;
    void exit() override;

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;
    bool handleMouseDoubleClickEvent(QMouseEvent *event) override;

private:
    /**
     * @brief 清理所有预览项
     */
    void clearPreviewItems();

    /**
     * @brief 根据 m_points 和当前鼠标位置更新预览
     */
    void updatePreview(const QPointF& currentMousePos);

    QList<QPointF> m_points;

    // 预览项
    QGraphicsPathItem* m_previewPath = nullptr; // 已固定的线段
    QGraphicsLineItem* m_previewLine = nullptr; // "橡皮筋" 线
};

#endif // POLYLINEDRAWINGSTATE_H
