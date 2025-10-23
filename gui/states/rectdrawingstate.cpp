#include "rectdrawingstate.h"
#include "gui/views/imageviewer.h"
#include "gui/items/annotationrectitem.h"
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QPen>
#include <QDebug>

RectDrawingState::RectDrawingState(ImageViewer* viewer, QObject *parent)
    : DrawingState(viewer, parent)
{
}

RectDrawingState::~RectDrawingState()
{
    clearPreview();
}

void RectDrawingState::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        startDrawing(m_viewer->mapToScene(event->pos()));
        event->accept();
    }
}

void RectDrawingState::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDrawing) {
        updatePreview(m_viewer->mapToScene(event->pos()));
        event->accept();
    }
}

void RectDrawingState::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        finishDrawing(m_viewer->mapToScene(event->pos()));
        event->accept();
    }
}

void RectDrawingState::startDrawing(const QPointF &scenePos)
{
    m_startPoint = scenePos;
    m_isDrawing = true;

    // 创建预览矩形
    m_previewRect = new QGraphicsRectItem(QRectF(m_startPoint, m_startPoint));
    QPen pen(Qt::yellow, 1, Qt::DashLine);
    m_previewRect->setPen(pen);
    m_previewRect->setBrush(Qt::transparent);
    m_previewRect->setZValue(1000);
    m_viewer->scene()->addItem(m_previewRect);

    qDebug() << "Rect drawing started by state machine";
}

void RectDrawingState::updatePreview(const QPointF &scenePos)
{
    if (m_previewRect) {
        m_previewRect->setRect(QRectF(m_startPoint, scenePos).normalized());
    }
}

void RectDrawingState::finishDrawing(const QPointF &scenePos)
{
    clearPreview();

    QRectF rect = QRectF(m_startPoint, scenePos).normalized();
    if (rect.isValid() && rect.width() > 1 && rect.height() > 1) {
        AnnotationRectItem *rectItem = new AnnotationRectItem(rect.x(), rect.y(), rect.width(), rect.height());
        m_viewer->scene()->addItem(rectItem);
        qDebug() << "Rect completed by state machine:" << rect;
    }

    m_isDrawing = false;
}

void RectDrawingState::clearPreview()
{
    if (m_previewRect) {
        m_viewer->scene()->removeItem(m_previewRect);
        delete m_previewRect;
        m_previewRect = nullptr;
    }
}
