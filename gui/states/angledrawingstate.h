#ifndef ANGLEDRAWINGSTATE_H
#define ANGLEDRAWINGSTATE_H

#include "drawingstate.h"
#include <QPointF>

class QGraphicsLineItem;
class QGraphicsTextItem;

/**
 * @brief 专用于处理 "三点点击" 绘制角度的状态
 */
class AngleDrawingState : public DrawingState
{
    Q_OBJECT
public:
    explicit AngleDrawingState(DrawingStateMachine* machine, QObject *parent = nullptr);
    virtual ~AngleDrawingState() override;

    void enter() override;
    void exit() override;

    bool handleMousePressEvent(QMouseEvent *event) override;
    bool handleMouseMoveEvent(QMouseEvent *event) override;
    bool handleMouseReleaseEvent(QMouseEvent *event) override;
    bool handleWheelEvent(QWheelEvent *event) override;

private:
    /**
     * @brief 清理所有预览项
     */
    void clearPreviewItems();

    /**
     * @brief 根据 m_pV 和当前鼠标位置更新预览
     */
    void updatePreview(const QPointF& currentMousePos);

    int m_clickCount = 0;
    QPointF m_pA; // 端点 A
    QPointF m_pV; // 顶点 V

    // 预览项
    QGraphicsLineItem* m_previewLineA = nullptr;
    QGraphicsLineItem* m_previewLineB = nullptr;
    QGraphicsTextItem* m_previewLabel = nullptr;
};

#endif // ANGLEDRAWINGSTATE_H
