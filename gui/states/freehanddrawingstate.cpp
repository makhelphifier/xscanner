#include "freehanddrawingstate.h"
#include "drawingstatemachine.h"
#include "views/imageviewer.h"
#include "items/freehandroi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPen>
#include <QDebug>

FreehandDrawingState::FreehandDrawingState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent), m_previewItem(nullptr)
{
}

FreehandDrawingState::~FreehandDrawingState()
{
    clearPreview();
}

void FreehandDrawingState::enter()
{
    qDebug() << "Entering FreehandDrawingState";
    m_isDrawing = false;
    m_currentPath = QPainterPath();
    clearPreview();
}

void FreehandDrawingState::exit()
{
    qDebug() << "Exiting FreehandDrawingState";
    clearPreview();
    m_isDrawing = false;
}

bool FreehandDrawingState::handleMousePressEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer || event->button() != Qt::LeftButton) {
        return false;
    }

    m_isDrawing = true;
    QPointF scenePos = viewer->mapToScene(event->pos());

    // 1. 开始新路径
    m_currentPath = QPainterPath();
    m_currentPath.moveTo(scenePos);

    // 2. 创建预览项
    clearPreview(); // 清理以防万一
    m_previewItem = new QGraphicsPathItem();
    QPen previewPen(Qt::yellow, 2, Qt::DashLine); // 使用虚线
    m_previewItem->setPen(previewPen);
    m_previewItem->setPath(m_currentPath);
    m_previewItem->setZValue(1001);
    viewer->scene()->addItem(m_previewItem);

    event->accept();
    return true;
}

bool FreehandDrawingState::handleMouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDrawing) {
        return false; // 没按下左键
    }

    QPointF scenePos = machine()->viewer()->mapToScene(event->pos());

    // 1. 将点添加到路径
    m_currentPath.lineTo(scenePos);

    // 2. 更新预览
    if (m_previewItem) {
        m_previewItem->setPath(m_currentPath);
    }

    event->accept();
    return true;
}

bool FreehandDrawingState::handleMouseReleaseEvent(QMouseEvent *event)
{
    if (!m_isDrawing || event->button() != Qt::LeftButton) {
        return false;
    }

    qDebug() << "FreehandDrawingState: Mouse released, finishing drawing.";
    m_isDrawing = false;

    // 1. 清理预览
    clearPreview();

    // 2. 检查路径是否有效
    if (m_currentPath.elementCount() < 3) {
        qDebug() << "FreehandDrawingState: Path too short, cancelling.";
        machine()->setState(DrawingStateMachine::Idle);
        return true;
    }

    // 3. 创建最终的 FreehandROI
    // 注意：FreehandROI 的构造函数会处理坐标系
    FreehandROI* roiItem = new FreehandROI(m_currentPath);

    // 4. (重要) 将其与 ROI 属性（边界、吸附等）关联
    // 我们仍然设置它们，即使 FreehandROI 尚未使用它们
    roiItem->setMaxBounds(machine()->viewer()->imageBounds());
    connect(roiItem, &ROI::extractRequested,
            machine()->viewer(), &ImageViewer::onExtractRegion);

    machine()->viewer()->scene()->addItem(roiItem);

    // 5. 返回 Idle
    machine()->setState(DrawingStateMachine::Idle);
    event->accept();
    return true;
}

bool FreehandDrawingState::handleWheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
    return true; // 绘制时吞掉滚轮事件
}

bool FreehandDrawingState::handleMouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return true; // 绘制时吞掉双击事件
}

void FreehandDrawingState::clearPreview()
{
    if (m_previewItem) {
        if (m_previewItem->scene()) {
            m_previewItem->scene()->removeItem(m_previewItem);
        }
        delete m_previewItem;
        m_previewItem = nullptr;
    }
}
