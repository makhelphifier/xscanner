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
#include "gui/states/genericdrawingstate.h"
#include "gui/items/infinitelineitem.h"
#include <QGraphicsScene>
#include "gui/items/angledlineroi.h"
#include "angledrawingstate.h"
#include "gui/items/pointmeasureitem.h"
#include "polylinedrawingstate.h"
#include <QTimer>
#include <QTextCursor>
#include "freehanddrawingstate.h"
#include "gui/items/annotationtextitem.h"
#include "gui/viewmodels/imageviewmodel.h"

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
        ImageViewer::ToolMode tool = viewer->currentToolMode();
        switch (tool) {
        case ImageViewer::ModeDrawRect:
        case ImageViewer::ModeDrawLine:
        case ImageViewer::ModeDrawAngledLine:
        case ImageViewer::ModeDrawEllipse:
        {
            if (machine()->startGenericDrawingState(tool, event)) {
                return true;
            }
            break;
        }
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
        case ImageViewer::ModeDrawAngle:
        {
            log_("BRANCH: Switching to AngleDrawingState");
            machine()->setState(DrawingStateMachine::AngleDrawing);
            return machine()->angleDrawingState()->handleMousePressEvent(event);
        }
        case ImageViewer::ModeDrawPolyline:
        {
            log_("BRANCH: Switching to PolylineDrawingState");
            machine()->setState(DrawingStateMachine::PolylineDrawing);
            // 转发第一次点击
            return machine()->polylineDrawingState()->handleMousePressEvent(event);
        }
        case ImageViewer::ModeDrawFreehand:
        {
            log_("BRANCH: Switching to FreehandDrawingState");
            machine()->setState(DrawingStateMachine::FreehandDrawing);
            return machine()->freehandDrawingState()->handleMousePressEvent(event);
        }
        case ImageViewer::ModeDrawText:
        {
            log_("BRANCH: Creating AnnotationTextItem");
            ImageViewer* viewer = machine()->viewer();
            QPointF scenePos = machine()->startDragPos();

            AnnotationTextItem* textItem = new AnnotationTextItem(scenePos);
            viewer->scene()->addItem(textItem);
            textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
            textItem->setFocus(Qt::MouseFocusReason);
            QTextCursor cursor = textItem->textCursor();
            cursor.select(QTextCursor::Document);
            textItem->setTextCursor(cursor);

            machine()->setState(DrawingStateMachine::Idle);
            event->accept();
            return true;
        }
        case ImageViewer::ModeDrawPoint:
        {
            ImageViewer* viewer = machine()->viewer();
            QPointF scenePos = machine()->startDragPos(); // 获取点击位置

            // 检查点击是否在图像内
            // if (!viewer->imageBounds().contains(scenePos)) {
            //     return true; // 消耗点击，但什么也不做
            // }
            if (!viewer->viewModel() || !viewer->viewModel()->imageBounds().contains(scenePos)) {
                return true;
            }

            // 创建新的点测量项
            PointMeasureItem* item = new PointMeasureItem(scenePos, viewer->viewModel());
            viewer->scene()->addItem(item);

            QTimer::singleShot(0, [viewer]() {
                if (viewer) {
                    viewer->scaleView(1.000001);
                    QTimer::singleShot(0, [viewer]() {
                        if (viewer) {
                            viewer->scaleView(1.0 / 1.000001);
                        }
                    });
                }
            });

            return true; // 事件已处理，保持在 IdleState
        }
        case ImageViewer::ModeSelect:
        default:
        {
            InfiniteLineItem* line = qgraphicsitem_cast<InfiniteLineItem*>(item);
            if (line && line->isMovable())
            {
                log_("IdleState: Detected movable InfiniteLineItem. Passing event to item.");
                return false;
            }
            if (item && (item->flags() & QGraphicsItem::ItemIsMovable))
            {
                log_("IdleState: Detected item with ItemIsMovable flag. Passing event to QGraphicsView.");
                return false;
            }
            machine()->setState(DrawingStateMachine::Panning);
            return true;
        }
        }
    }

    return false;
}


bool IdleState::handleMouseMoveEvent(QMouseEvent *event)
{
    // Idle 状态下，鼠标移动通常只用于更新像素信息，由 ImageViewer 处理
    // ImageViewer* viewer = machine()->viewer();
    // if (viewer) {
    //     viewer->updatePixelInfo(viewer->mapToScene(event->pos()));
    // }
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
