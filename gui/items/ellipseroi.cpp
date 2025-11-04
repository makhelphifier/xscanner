#include "ellipseroi.h"
#include "handle.h" //
#include <QPainter>
#include <QPainterPath>

// 构造函数1: (用于代码创建，或被构造函数2调用)
// 这个函数体完全复制自 RectROI::RectROI
EllipseROI::EllipseROI(const QPointF& pos, const QSizeF& size, QGraphicsItem* parent)
    : ROI(pos, size, parent) //
{
    // [逻辑同 RectROI]
    // 添加四个角的 scale handle（对角缩放）
    addScaleHandle({0.0, 0.0}, {1.0, 1.0});
    addScaleHandle({1.0, 0.0}, {0.0, 1.0});  // 右下角, 锚点: 左上角
    addScaleHandle({0.0, 1.0}, {1.0, 0.0});  // 左上角, 锚点: 右下角
    addScaleHandle({1.0, 1.0}, {0.0, 0.0});  // 右上角, 锚点: 左下角
    // 添加四个边中点的 scale handle（对边缩放）
    addScaleHandle({1.0, 0.5}, {0.0, 0.5});  // 右中, 锚点: 左中
    addScaleHandle({0.5, 1.0}, {0.5, 0.0}); // 上中, 锚点: 下中
    addScaleHandle({0.0, 0.5}, {1.0, 0.5}); // 左中, 锚点: 右中
    addScaleHandle({0.5, 0.0}, {0.5, 1.0}); // 下中, 锚点: 上中
    // 旋转手柄 (旋转中心总是 0.5, 0.5)
    addRotateHandle({0.5, -0.1}, {0.5, 0.5});
}

// 构造函数2: (用于 GenericDrawingState)
// [Ctrl 键约束逻辑]
EllipseROI::EllipseROI(const QPointF &startPoint, const QPointF &endPoint, QGraphicsItem *parent)
    // 1. 调用静态辅助函数计算矩形（处理Ctrl键）
    : EllipseROI(constrainRectToSquare(startPoint, endPoint).normalized().topLeft(),
                 constrainRectToSquare(startPoint, endPoint).normalized().size(),
                 parent)
{
    // 构造函数1 已经完成了所有句柄的添加
}


// [关键重写]
void EllipseROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);

    // m_currentPen 是从基类 ROI 继承的
    QPen p = m_currentPen;
    p.setWidth(2); // 确保线宽
    painter->setPen(p);

    // [修改] 不画矩形，画椭圆
    // 基类 ROI::paint() 画的是 painter->drawRect(boundingRect());
    painter->drawEllipse(boundingRect());
}

// [关键重写]
QPainterPath EllipseROI::shape() const
{
    // 基类 ROI::shape() 返回的是矩形
    // 我们必须返回椭圆，这样点击检测才准确
    QPainterPath path;
    path.addEllipse(boundingRect());

    // （可选）添加句柄的 shape 以便选中
    for (const auto& handleInfo : qAsConst(m_handles)) {
        if(handleInfo.item) {
            path.addPath(handleInfo.item->mapToParent(handleInfo.item->shape()));
        }
    }

    return path;
}
