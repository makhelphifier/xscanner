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
#include "gui/items/rectroi.h"
#include "gui/items/linesegmentroi.h"
#include "gui/states/genericdrawingstate.h"
#include "gui/items/infinitelineitem.h"
#include <QGraphicsScene>

IdleState::IdleState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent) {}

bool IdleState::handleMousePressEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer) {
        return false;
    }
    QPointF scenePos = viewer->mapToScene(event->pos());
    machine()->setLastMousePos(event->pos());
    machine()->setStartDragPos(scenePos);

    QGraphicsItem* item = viewer->itemAt(event->pos());

    Handle* handle = qgraphicsitem_cast<Handle*>(item);


    ROI* roi = nullptr;
    QGraphicsItem* currentItem = item;
    int loopCount = 0;
    while (currentItem && !roi) {
        roi = dynamic_cast<ROI*>(currentItem);

        currentItem = currentItem->parentItem();
        loopCount++;
    }
    if (handle && event->button() == Qt::LeftButton) {
        machine()->startDraggingHandle(handle, scenePos);
        machine()->setState(DrawingStateMachine::DraggingHandle);
        return true;
    }
    // --- 优先级 2: 拖动 InfiniteLineItem (新增) ---
    // InfiniteLineItem* line = qgraphicsitem_cast<InfiniteLineItem*>(item);
    // if (line && event->button() == Qt::LeftButton) {
    //     // 找到了线条，并且是左键点击。
    //     // 我们不在这里处理它，而是返回 false，
    //     // QGraphicsView 会自动将此事件传递给 line item 自己的 mousePressEvent。
    //     return false;
    // }

    if (roi && event->button() == Qt::LeftButton && item == roi) {
        return false;
    }

    if (event->button() == Qt::LeftButton) {
        // 1. 从 viewer 获取当前工具模式
        ImageViewer::ToolMode tool = viewer->currentToolMode();

        DrawingState* nextState = nullptr; // 准备一个指针
        bool isTemporary = false;          // 标记是否为临时

        // 2. 使用 switch 决定下一个状态
        switch (tool) {
        case ImageViewer::ModeDrawRect:
            log_("BRANCH: Creating GenericDrawingState<RectROI>");
            nextState = new GenericDrawingState<RectROI>(machine());
            isTemporary = true;
            break;
        case ImageViewer::ModeDrawLine:
            log_("BRANCH: Creating GenericDrawingState<LineSegmentROI>");
            nextState = new GenericDrawingState<LineSegmentROI>(machine());
            isTemporary = true;
            break;
        case ImageViewer::ModeDrawHLine:
        case ImageViewer::ModeDrawVLine:
        {
            qreal angle = (tool == ImageViewer::ModeDrawHLine) ? 0 : 90;
            InfiniteLineItem* line = new InfiniteLineItem(viewer, angle);
            viewer->scene()->addItem(line);
            qreal value = (angle == 0) ? scenePos.y() : scenePos.x();
            line->setValue(value);
            machine()->setState(DrawingStateMachine::Idle);
            return true; // 事件已处理
        }
        case ImageViewer::ModeSelect:
        default:
        {
            // // 仅在选择/默认模式下，检查是否点击了可拖动的线条
            // InfiniteLineItem* line = qgraphicsitem_cast<InfiniteLineItem*>(item);
            // if (line && line->isMovable())
            // {
            //     // 是线条，返回 false，让 QGraphicsView 将事件传递给 line
            //     return false;
            // }

            // 不是线条（或线条不可拖动），则开始平移 (Panning)
            machine()->setState(DrawingStateMachine::Panning);
            return true;}
        }

        if (nextState) {
            machine()->setState(nextState, isTemporary);
            return nextState->handleMousePressEvent(event);
        }
    }

    return false;
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
