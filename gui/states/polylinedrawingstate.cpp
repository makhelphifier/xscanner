#include "polylinedrawingstate.h"
#include "drawingstatemachine.h"
#include "gui/views/imageviewer.h"
#include "gui/items/polylineroi.h" // 包含我们刚创建的 ROI
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QPen>
#include <QDebug>
#include <QWheelEvent>

PolylineDrawingState::PolylineDrawingState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent), m_previewPath(nullptr), m_previewLine(nullptr)
{
}

PolylineDrawingState::~PolylineDrawingState()
{
    clearPreviewItems();
}

void PolylineDrawingState::enter()
{
    qDebug() << "Entering PolylineDrawingState";
    m_points.clear();
    clearPreviewItems();
}

void PolylineDrawingState::exit()
{
    qDebug() << "Exiting PolylineDrawingState";
    clearPreviewItems();
    m_points.clear();
}

bool PolylineDrawingState::handleMousePressEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer) return false;

    // 右键点击 = 取消
    if (event->button() == Qt::RightButton) {
        machine()->setState(DrawingStateMachine::Idle);
        return true;
    }

    if (event->button() != Qt::LeftButton) {
        return false; // 只响应左键
    }

    QPointF scenePos = viewer->mapToScene(event->pos());
    m_points.append(scenePos);

    if (m_points.size() == 1) { // 第一次点击
        // 1. 创建 "橡皮筋" 线
        QPen previewPen(Qt::yellow, 1, Qt::DashLine);
        m_previewLine = new QGraphicsLineItem(QLineF(scenePos, scenePos));
        m_previewLine->setPen(previewPen);
        m_previewLine->setZValue(1001); // 确保在最上层
        viewer->scene()->addItem(m_previewLine);

        // 2. 创建预览路径 (用于显示已固定的线段)
        QPen pathPen(Qt::yellow, 2, Qt::SolidLine);
        m_previewPath = new QGraphicsPathItem();
        m_previewPath->setPen(pathPen);
        m_previewPath->setZValue(1000);
        viewer->scene()->addItem(m_previewPath);
    }

    // 更新预览（这会绘制新添加的点）
    updatePreview(scenePos);

    event->accept();
    return true;
}

bool PolylineDrawingState::handleMouseMoveEvent(QMouseEvent *event)
{
    if (m_points.isEmpty() || !m_previewLine) {
        return false; // 还没开始
    }

    QPointF scenePos = machine()->viewer()->mapToScene(event->pos());

    // 只更新 "橡皮筋" 线
    m_previewLine->setLine(QLineF(m_points.last(), scenePos));

    event->accept();
    return true;
}

bool PolylineDrawingState::handleMouseDoubleClickEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer || event->button() != Qt::LeftButton || m_points.isEmpty()) {
        return false; // 必须是左键双击，并且至少已点击一次
    }

    qDebug() << "PolylineDrawingState: Double-click detected, finishing drawing.";

    // 1. 添加双击的这个点作为最后一个点
    QPointF scenePos = viewer->mapToScene(event->pos());
    m_points.append(scenePos);

    // 2. 至少需要2个点才能创建
    if (m_points.size() < 2) {
        machine()->setState(DrawingStateMachine::Idle); // 点太少，取消
        return true;
    }

    // 3. 创建最终的 PolylineROI
    PolylineROI* roiItem = new PolylineROI(m_points);

    // 4. (重要) 将其与 ROI 属性（边界、吸附等）关联
    roiItem->setMaxBounds(viewer->imageBounds());
    roiItem->setTranslateSnap(1.0); // 1 像素吸附
    connect(roiItem, &ROI::extractRequested,
            viewer, &ImageViewer::onExtractRegion);

    viewer->scene()->addItem(roiItem);

    // 5. 清理预览项并返回 Idle 状态
    clearPreviewItems();
    m_points.clear();
    machine()->setState(DrawingStateMachine::Idle);

    event->accept();
    return true;
}


bool PolylineDrawingState::handleMouseReleaseEvent(QMouseEvent *event)
{
    // 在这个状态机中，单击释放事件不做任何事（只在 Press 时添加点）
    // 我们返回 true 来 "消耗" 它，防止它被传递
    return true;
}

bool PolylineDrawingState::handleWheelEvent(QWheelEvent *event)
{
    // 绘制时忽略滚轮
    qDebug() << "PolylineDrawingState: Ignoring wheel event.";
    return true;
}

void PolylineDrawingState::clearPreviewItems()
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer || !viewer->scene()) return;

    QGraphicsScene* scene = viewer->scene();

    if (m_previewPath) {
        scene->removeItem(m_previewPath);
        delete m_previewPath;
        m_previewPath = nullptr;
    }
    if (m_previewLine) {
        scene->removeItem(m_previewLine);
        delete m_previewLine;
        m_previewLine = nullptr;
    }
}

void PolylineDrawingState::updatePreview(const QPointF& currentMousePos)
{
    if (m_points.isEmpty()) return;

    // 1. 更新 "橡皮筋" 线
    if (m_previewLine) {
        m_previewLine->setLine(QLineF(m_points.last(), currentMousePos));
    }

    // 2. 更新已固定的路径
    if (m_previewPath) {
        QPainterPath path;
        path.moveTo(m_points.first());
        for (int i = 1; i < m_points.size(); ++i) {
            path.lineTo(m_points[i]);
        }
        m_previewPath->setPath(path);
    }
}
