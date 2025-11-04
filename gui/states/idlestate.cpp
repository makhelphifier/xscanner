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
#include "gui/items/angledlineroi.h"
#include "angledrawingstate.h"
#include "gui/items/pointmeasureitem.h"
#include <QTimer>

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
        case ImageViewer::ModeDrawAngledLine:
            log_("BRANCH: Creating GenericDrawingState<AngledLineROI>");
            nextState = new GenericDrawingState<AngledLineROI>(machine());
            isTemporary = true;
            break;
        case ImageViewer::ModeDrawAngle:
        {
            log_("BRANCH: Switching to AngleDrawingState");
            machine()->setState(DrawingStateMachine::AngleDrawing);
            return machine()->angleDrawingState()->handleMousePressEvent(event);
        }
        case ImageViewer::ModeDrawPoint:
        {
            ImageViewer* viewer = machine()->viewer();
            QPointF scenePos = machine()->startDragPos(); // 获取点击位置

            // 检查点击是否在图像内
            if (!viewer->imageBounds().contains(scenePos)) {
                return true; // 消耗点击，但什么也不做
            }

            // 创建新的点测量项
            PointMeasureItem* item = new PointMeasureItem(scenePos, viewer);
            viewer->scene()->addItem(item);
            // --- 2. "取巧"的解决方案 (Hide/Show Hack) ---
            // 立即隐藏该项
            // item->hide();
            // 安排它在下一个事件循环中显示。
            // 这会强制 QGraphicsScene 重新计算其几何形状
            // --- "取巧"的解决方案 (Zoom Hack) ---
            // 按照您的要求，在创建后模拟一次缩放
            // 我们使用一个0ms的定时器来确保这个操作在
            // "addItem" 完成 *之后* 的下一个事件循环中执行
            QTimer::singleShot(0, [viewer]() {
                if (viewer) {
                    // 1. 施加一个几乎为零的缩放
                    viewer->scaleView(1.000001);

                    // 2. 立即在 *另*一个 0ms 定时器中缩放回来
                    //    这确保了重绘事件在两次缩放之间被处理
                    QTimer::singleShot(0, [viewer]() {
                        if (viewer) {
                            viewer->scaleView(1.0 / 1.000001);
                        }
                    });
                }
            });
            // --- 结束 Hack ---
            return true; // 事件已处理，保持在 IdleState
        }
        case ImageViewer::ModeSelect:
        default:
        {
            // 仅在选择/默认模式下，检查是否点击了可拖动的线条
            InfiniteLineItem* line = qgraphicsitem_cast<InfiniteLineItem*>(item);
            if (line && line->isMovable())
            {
                log_("sss");
                // 是线条，返回 false，让 QGraphicsView 将事件传递给 line
                return false;
            }

            // 不是线条（或线条不可拖动），则开始平移 (Panning)
            machine()->setState(DrawingStateMachine::Panning);
            return true;
        }

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
