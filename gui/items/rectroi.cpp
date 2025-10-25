// rectroi.cpp

#include "rectroi.h"
#include "handle.h" // 创建句柄需要包含此头文件

// 构造函数1的实现
RectROI::RectROI(const QPointF& pos, const QSizeF& size, bool centered, bool sideScalers, QGraphicsItem* parent)
    : ROI(pos, size, parent)
{
    QPointF center = centered ? QPointF(0.5, 0.5) : QPointF(0.0, 0.0);
    addScaleHandle({1.0, 1.0}, center);
    if (sideScalers) {
        addScaleHandle({1.0, 0.5}, {center.x(), 0.5});
        addScaleHandle({0.5, 1.0}, {0.5, center.y()});
    }
}

// 新增：构造函数2的实现
RectROI::RectROI(const QPointF &startPoint, const QPointF &endPoint, QGraphicsItem *parent)
    // 使用 QRectF::normalized() 来确保 pos 是左上角，size 是正值
    : RectROI(QRectF(startPoint, endPoint).normalized().topLeft(),
              QRectF(startPoint, endPoint).normalized().size(),
              false, // 默认不居中
              false, // 默认不带边缩放
              parent)
{
    // 构造函数代理已经完成了所有工作
}
