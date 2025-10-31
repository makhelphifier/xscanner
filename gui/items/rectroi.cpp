#include "rectroi.h"
#include "handle.h"

RectROI::RectROI(const QPointF& pos, const QSizeF& size, bool centered, bool sideScalers, QGraphicsItem* parent)
    : ROI(pos, size, parent)
{
    // QPointF center = centered ? QPointF(0.5, 0.5) : QPointF(0.0, 0.0);

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

RectROI::RectROI(const QPointF &startPoint, const QPointF &endPoint, QGraphicsItem *parent)
    : RectROI(QRectF(startPoint, endPoint).normalized().topLeft(),
              QRectF(startPoint, endPoint).normalized().size(),
              false,
              false,
              parent)
{
}
