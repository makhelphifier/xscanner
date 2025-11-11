#ifndef GENERICDRAWINGSTATE_H
#define GENERICDRAWINGSTATE_H

#include "drawingstate.h"
#include "items/rectroi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QPointF>
#include <QDebug>
#include <QGraphicsItem>
#include "states/drawingstatemachine.h"
#include "views/imageviewer.h"
#include "items/roi.h"
#include "items/angledlineroi.h"

class ImageViewer;
class AngledLineROI;
class DrawingStateMachine;
template <typename T>
struct has_preview_item_type {
    template <typename U>
    static auto test(int) -> decltype(typename U::PreviewItemType{ }, std::true_type{ });
    template <typename>
    static std::false_type test(...);
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<class FinalItem>
class GenericDrawingState : public DrawingState
{
public:
    explicit GenericDrawingState(DrawingStateMachine* machine, QObject *parent = nullptr)
        : DrawingState(machine, parent), m_isDrawing(false), m_previewItem(nullptr)
    {
    }

    ~GenericDrawingState() override
    {
        clearPreview();
    }

    bool handleMousePressEvent(QMouseEvent *event) override
    {
        ImageViewer* viewer = machine()->viewer(); // 通过状态机获取 viewer
        if (!viewer) return false;

        if (event->button() == Qt::LeftButton) { // Qt::LeftButton 而非 Qt::MouseButton::LeftButton
            m_startPoint = viewer->mapToScene(event->pos());

            m_isDrawing = true;

            if constexpr (has_preview_item_type<FinalItem>::value) { // C++17 if constexpr + SFINAE
                m_previewItem = new typename FinalItem::PreviewItemType();
                if (m_previewItem) {
                    FinalItem::initPreview(m_previewItem);
                    FinalItem::updatePreview(m_previewItem, m_startPoint, m_startPoint);
                    viewer->scene()->addItem(m_previewItem);
                } else {
                    qWarning() << "Failed to create preview item.";
                    m_isDrawing = false; // 创建失败则取消绘制
                    return false;
                }
            }else {
                qWarning() << "FinalItem does not define PreviewItemType.";
                m_isDrawing = false; // 无法创建预览则取消绘制
                return false;
            }
            event->accept();
            return true; // 事件已处理
        }
        return false; // 其他按钮不处理
    }

    bool handleMouseMoveEvent(QMouseEvent *event) override
    {
        ImageViewer* viewer = machine()->viewer();
        if (!viewer) return false;

        if (m_isDrawing && m_previewItem) {
            // 使用 FinalItem 中的静态方法更新预览
            FinalItem::updatePreview(m_previewItem, m_startPoint, viewer->mapToScene(event->pos()));
            event->accept();
            return true; // 事件已处理
        }
        return false; // 未处理
    }

    bool handleMouseReleaseEvent(QMouseEvent *event) override
    {
        ImageViewer* viewer = machine()->viewer();
        if (!viewer) return false;

        bool wasDrawing = m_isDrawing; // 记录是否正在绘制
        m_isDrawing = false; // 先停止绘制状态

        if (event->button() == Qt::LeftButton && wasDrawing) {
            clearPreview(); // 清理预览项

            QPointF endPoint = viewer->mapToScene(event->pos());

            // 检查绘制是否有效 (例如，距离是否足够大)
            if ((endPoint - m_startPoint).manhattanLength() > 5) {
                // 使用 FinalItem 的构造函数创建最终项
                FinalItem* finalItem = new FinalItem(m_startPoint, endPoint);
                // 尝试将新项转换为 ROI
                if (ROI* roiItem = dynamic_cast<ROI*>(finalItem)) {
                    // 如果转换成功，从 viewer 获取边界并设置
                    roiItem->setMaxBounds(viewer->imageBounds());
                    // 设置吸附值
                    roiItem->setTranslateSnap(50.0); // 吸附到 1 像素
                    roiItem->setScaleSnap(50.0);     // 吸附到 1 像素
                    roiItem->setRotateSnap(15.0);   // 吸附到 15 度
                    connect(roiItem, &ROI::extractRequested,
                            machine()->viewer(), &ImageViewer::onExtractRegion);
                    qDebug() << "GenericDrawingState: Applied maxBounds to new ROI.";
                }
                if (AngledLineROI* angledLine = dynamic_cast<AngledLineROI*>(finalItem)) {
                    angledLine->setViewer(viewer);
                    qDebug() << "GenericDrawingState: Linked AngledLineROI to viewer.";
                }
                viewer->scene()->addItem(finalItem);
                qDebug() << "GenericDrawingState: Created FinalItem.";
            } else {
                qDebug() << "GenericDrawingState: Drawing too small, cancelled.";
            }

            event->accept();
            machine()->setState(DrawingStateMachine::Idle);
            return true; // 事件已处理
        }
        return false;
    }

    bool handleWheelEvent(QWheelEvent *event) override
    {
        Q_UNUSED(event);
        qDebug() << "GenericDrawingState: Wheel Event ignored";
        return true;
    }


private:
    void clearPreview()
    {
        ImageViewer* viewer = machine()->viewer();
        if (m_previewItem && viewer && viewer->scene()) {
            viewer->scene()->removeItem(m_previewItem);
            delete m_previewItem;
            m_previewItem = nullptr;
        }
    }

    bool m_isDrawing;
    QPointF m_startPoint;
    // 使用指针类型别名简化代码
    using PreviewItem = typename FinalItem::PreviewItemType;
    PreviewItem* m_previewItem;

};

#endif // GENERICDRAWINGSTATE_H
