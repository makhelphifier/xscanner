#ifndef GENERICDRAWINGSTATE_H
#define GENERICDRAWINGSTATE_H

#include "drawingstate.h"
#include "../views/imageviewer.h"
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QPointF>
#include <QDebug>

template<class FinalItem>
class GenericDrawingState : public DrawingState
{
public:
    explicit GenericDrawingState(ImageViewer* viewer, QObject *parent = nullptr)
        : DrawingState(viewer, parent), m_isDrawing(false), m_previewItem(nullptr)
    {
    }

    ~GenericDrawingState() override
    {
        clearPreview();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_startPoint = m_viewer->mapToScene(event->pos());
            m_isDrawing = true;

            // 创建预览项
            m_previewItem = new typename FinalItem::PreviewItemType();
            FinalItem::initPreview(m_previewItem); // 使用 FinalItem 中的静态方法初始化预览
            m_viewer->scene()->addItem(m_previewItem);

            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_isDrawing && m_previewItem) {
            // 使用 FinalItem 中的静态方法更新预览
            FinalItem::updatePreview(m_previewItem, m_startPoint, m_viewer->mapToScene(event->pos()));
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && m_isDrawing) {
            clearPreview();

            QPointF endPoint = m_viewer->mapToScene(event->pos());
            // 使用 FinalItem 的构造函数创建最终项
            FinalItem* finalItem = new FinalItem(m_startPoint, endPoint);
            m_viewer->scene()->addItem(finalItem);

            m_isDrawing = false;
            event->accept();
        }
    }

private:
    void clearPreview()
    {
        if (m_previewItem) {
            m_viewer->scene()->removeItem(m_previewItem);
            delete m_previewItem;
            m_previewItem = nullptr;
        }
    }

    bool m_isDrawing;
    QPointF m_startPoint;
    typename FinalItem::PreviewItemType* m_previewItem;
};

#endif // GENERICDRAWINGSTATE_H
