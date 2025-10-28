#ifndef GENERICDRAWINGSTATE_H
#define GENERICDRAWINGSTATE_H

#include "drawingstate.h"
#include "gui/items/rectroi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QPointF>
#include <QDebug>
#include <QGraphicsItem>
#include "gui/states/drawingstatemachine.h"
#include "gui/views/imageviewer.h"
// 前向声明 ImageViewer
class ImageViewer;
class DrawingStateMachine; // 需要包含或前向声明状态机
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
    // ++ 修改构造函数，接收 DrawingStateMachine* ++
    explicit GenericDrawingState(DrawingStateMachine* machine, QObject *parent = nullptr)
        : DrawingState(machine, parent), m_isDrawing(false), m_previewItem(nullptr)
    {
    }

    ~GenericDrawingState() override
    {
        clearPreview();
    }

    // ++ 修改返回类型为 bool，添加 override ++
    bool handleMousePressEvent(QMouseEvent *event) override
    {
        ImageViewer* viewer = machine()->viewer(); // 通过状态机获取 viewer
        if (!viewer) return false;

        if (event->button() == Qt::LeftButton) { // Qt::LeftButton 而非 Qt::MouseButton::LeftButton
            m_startPoint = viewer->mapToScene(event->pos());

            m_isDrawing = true;

            // 创建预览项 - 假设 FinalItem 有 PreviewItemType 定义和静态方法
            // 你需要确保 FinalItem 确实有这些
            if constexpr (has_preview_item_type<FinalItem>::value) { // C++17 if constexpr + SFINAE
                // 假设 FinalItem 定义了 createPreview 和 initPreview
                m_previewItem = FinalItem::createPreview(); // 静态方法创建
                if (m_previewItem) {
                    FinalItem::initPreview(m_previewItem, m_startPoint); // 静态方法初始化
                    viewer->scene()->addItem(m_previewItem);
                } else {
                    qWarning() << "Failed to create preview item.";
                    m_isDrawing = false; // 创建失败则取消绘制
                    return false;
                }
            } else {
                qWarning() << "FinalItem does not define PreviewItemType.";
                m_isDrawing = false; // 无法创建预览则取消绘制
                return false;
            }


            event->accept();
            return true; // 事件已处理
        }
        return false; // 其他按钮不处理
    }

    // ++ 修改返回类型为 bool，添加 override ++
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

    // ++ 修改返回类型为 bool，添加 override ++
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
            if ((endPoint - m_startPoint).manhattanLength() > 5) { // 示例：最小绘制距离
                // 使用 FinalItem 的构造函数创建最终项
                FinalItem* finalItem = new FinalItem(m_startPoint, endPoint); // 假设构造函数是这样
                viewer->scene()->addItem(finalItem);
                qDebug() << "GenericDrawingState: Created FinalItem.";
            } else {
                qDebug() << "GenericDrawingState: Drawing too small, cancelled.";
            }

            event->accept();
            machine()->setState(DrawingStateMachine::Idle); // ++ 返回 Idle 状态 ++
            return true; // 事件已处理
        }
        // 如果不是左键释放或者之前没有在绘制，则返回 Idle 状态（如果当前不是 Idle 的话）
        // 这一步通常由状态机的 handleMouseReleaseEvent 统一处理，这里不需要重复
        // machine()->setState(DrawingStateMachine::Idle);
        return false; // 未处理
    }

    // ++ 新增：实现 handleWheelEvent，添加 override ++
    bool handleWheelEvent(QWheelEvent *event) override
    {
        // 在绘制状态下通常忽略滚轮事件，但消耗掉它
        Q_UNUSED(event);
        qDebug() << "GenericDrawingState: Wheel Event ignored";
        return true; // 消耗事件，防止视图缩放
    }


private:
    void clearPreview()
    {
        ImageViewer* viewer = machine()->viewer();
        if (m_previewItem && viewer && viewer->scene()) { // 增加 viewer 和 scene 的检查
            viewer->scene()->removeItem(m_previewItem);
            delete m_previewItem;
            m_previewItem = nullptr;
        }
    }

    bool m_isDrawing;
    QPointF m_startPoint;
    // 使用指针类型别名简化代码
    using PreviewItem = typename FinalItem::PreviewItemType; // 假设 FinalItem 定义了 PreviewItemType
    PreviewItem* m_previewItem;

    // ImageViewer* m_viewer; // 不需要，通过 machine()->viewer() 获取
};

#endif // GENERICDRAWINGSTATE_H
