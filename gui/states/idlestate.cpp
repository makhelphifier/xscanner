// gui/states/idlestate.cpp
#include "idlestate.h"
#include "drawingstatemachine.h"
#include "gui/views/imageviewer.h"
#include "gui/items/handle.h"
#include "gui/items/roi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsItem>
#include <QDebug>
#include <QGraphicsLineItem>
#include "util/logger/logger.h"

IdleState::IdleState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent) {}

bool IdleState::handleMousePressEvent(QMouseEvent *event)
{
    log_("AAA --- IdleState::handleMousePressEvent ---");

    ImageViewer* viewer = machine()->viewer();
    if (!viewer) {
        log_("ERROR: viewer() is null.");
        return false;
    }

    // --- 诊断日志 1: 检查鼠标按键 ---
    if (event->button() == Qt::LeftButton) {
        log_("DIAG: Mouse button is Qt::LeftButton.");
    } else if (event->button() == Qt::RightButton) {
        log_("DIAG: Mouse button is Qt::RightButton.");
    } else if (event->button() == Qt::MiddleButton) {
        log_("DIAG: Mouse button is Qt::MiddleButton.");
    } else {
        log_("DIAG: Mouse button is OTHER.");
    }

    QPointF scenePos = viewer->mapToScene(event->pos());
    machine()->setLastMousePos(event->pos());
    machine()->setStartDragPos(scenePos);

    // --- 诊断日志 2: 检查 itemAt ---
    QGraphicsItem* item = viewer->itemAt(event->pos());
    if (item == nullptr) {
        log_("DIAG: itemAt(pos) returned nullptr (clicked empty space).");
    } else {
        // 尝试获取 item 的类名
        log_(QString("DIAG: itemAt(pos) returned an item. Type: %1").arg(item->type()));
        log_(QString("DIAG: itemAt(pos) returned an item. Type QGraphicsRectItem: %1").arg(QGraphicsRectItem::Type));
        log_(QString("DIAG: itemAt(pos) returned an item. Type Handle: %1").arg(Handle::Type));
        // log_(QGraphicsLineItem::type());
    }

    Handle* handle = qgraphicsitem_cast<Handle*>(item);
    if(handle){
        log_(QString("11111111111DIAG: qgraphicsitem_cast<Handle*>(item) is %1.")     .arg(handle ? "NOT null" : "null"));

    }
    log_(QString("DIAG: qgraphicsitem_cast<Handle*>(item) is %1.")
             .arg(handle ? "NOT null" : "null"));

    // --- 诊断日志 3: 检查 ROI 循环 ---
    ROI* roi = nullptr;
    QGraphicsItem* currentItem = item;
    int loopCount = 0;
    while (currentItem && !roi) {
        log_(QString("DIAG: Loop %1: Checking item of type %2 for ROI.")
                 .arg(loopCount)
                 .arg(currentItem->type()));
        roi = qgraphicsitem_cast<ROI*>(currentItem);
        if (roi) {
            log_("DIAG: Found ROI!");
        }
        currentItem = currentItem->parentItem();
        loopCount++;
    }
    log_(QString("DIAG: After loop, roi variable is %1.")
             .arg(roi ? "NOT null" : "null"));


    // --- 检查逻辑分支 ---

    if (handle && event->button() == Qt::LeftButton) {
        log_("BRANCH: Entering Handle drag branch."); // <--- 分支 1
        machine()->startDraggingHandle(handle, scenePos);
        machine()->setState(DrawingStateMachine::DraggingHandle);
        return true;
    }

    if (roi && event->button() == Qt::LeftButton && item == roi) { // 点击ROI本体
        log_("BRANCH: Entering ROI drag branch (returning false)."); // <--- 分支 2
        qDebug() << "IdleState: Clicked on ROI Body, letting ROI handle drag.";
        return false;
    }

    if (event->button() == Qt::LeftButton) {
        log_("BRANCH: Entering LeftButton (Panning) branch."); // <--- 分支 3 (你期望的)
        log_("BBB"); // <--- 你的日志
        machine()->setState(DrawingStateMachine::Panning);
        qDebug() << "IdleState: Left click on background, entering Panning state.";
        return true;
    }

    log_("BRANCH: No branch matched. Returning false."); // <--- 分支 4 (崩溃路径)
    return false; // 其他情况（例如右键点击背景）
}



bool IdleState::handleMouseMoveEvent(QMouseEvent *event)
{
    // Idle 状态下，鼠标移动通常只用于更新像素信息，由 ImageViewer 处理
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        viewer->updatePixelInfo(viewer->mapToScene(event->pos()));
    }
    return false; // 不消耗事件
}

bool IdleState::handleMouseReleaseEvent(QMouseEvent *event)
{
    // 在 Idle 状态下释放鼠标，通常不需要做特别处理
    return false;
}

bool IdleState::handleWheelEvent(QWheelEvent *event)
{
    // 在 Idle 状态下，滚轮用于缩放视图
    ImageViewer* viewer = machine()->viewer();
    if (viewer) {
        qreal scaleFactor = qPow(1.15, event->angleDelta().y() / 120.0);
        viewer->scaleView(scaleFactor);
        return true; // 事件已处理
    }
    return false;
}
