#ifndef RECTDRAWINGSTATE_H
#define RECTDRAWINGSTATE_H

#include "drawingstate.h"
#include <QPointF>

class QGraphicsRectItem;

class RectDrawingState : public DrawingState
{
    Q_OBJECT

public:
    explicit RectDrawingState(ImageViewer* viewer, QObject *parent = nullptr);
    ~RectDrawingState();

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_isDrawing = false;
    QPointF m_startPoint;
    QGraphicsRectItem* m_previewRect = nullptr;

    void startDrawing(const QPointF &scenePos);
    void updatePreview(const QPointF &scenePos);
    void finishDrawing(const QPointF &scenePos);
    void clearPreview();
};

#endif // RECTDRAWINGSTATE_H
