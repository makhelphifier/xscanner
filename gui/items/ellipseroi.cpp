#include "ellipseroi.h"
#include "handle.h"
#include <QPainter>
#include <QPainterPath>

EllipseROI::EllipseROI(const QPointF& pos, const QSizeF& size, QGraphicsItem* parent)
    : ROI(pos, size, parent) //
{
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

EllipseROI::EllipseROI(const QPointF &startPoint, const QPointF &endPoint, QGraphicsItem *parent)
    : EllipseROI(constrainRectToSquare(startPoint, endPoint).normalized().topLeft(),
                 constrainRectToSquare(startPoint, endPoint).normalized().size(),
                 parent)
{
}

void EllipseROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen p = m_currentPen;
    p.setWidth(2);
    painter->setPen(p);
    painter->drawEllipse(boundingRect());
}

QPainterPath EllipseROI::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    for (const auto& handleInfo : qAsConst(m_handles)) {
        if(handleInfo.item) {
            path.addPath(handleInfo.item->mapToParent(handleInfo.item->shape()));
        }
    }
    return path;
}
