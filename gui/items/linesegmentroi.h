#ifndef LINESEGMENTROI_H
#define LINESEGMENTROI_H

#include "roi.h"
#include <QGraphicsLineItem> // 用于预览
#include <QPen> // 用于 initPreview

/**
 * @brief 由两个自由句柄定义的线段ROI。
 *
 * 继承自ROI，但其形状由两个Handle的位置定义，
 * 而不是由ROI的pos()和size()定义。
 */
class LineSegmentROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数: 用于代码直接创建或 GenericDrawingState (点在场景坐标系中)
    inline explicit LineSegmentROI(const QPointF& startPoint, const QPointF& endPoint, QGraphicsItem* parent = nullptr)
        : ROI(QPointF(0,0), QSizeF(1,1), parent)
    {
        // 现在直接在头文件中调用 initialize
        initialize(startPoint, endPoint);
    }

    virtual ~LineSegmentROI() override = default;

    enum { Type = UserType + 4 }; // 假设 UserType+3 已被 RectROI 使用
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true) override;

    // --- 适配 GenericDrawingState ---
    // 预览时使用一个简单的虚线
    using PreviewItemType = QGraphicsLineItem;

    static void initPreview(PreviewItemType* item) {
        QPen pen(Qt::yellow, 1, Qt::DashLine);
        item->setPen(pen);
        // QGraphicsLineItem 没有 setBrush
        // item->setBrush(Qt::transparent);
        item->setZValue(1000);
    }

    static void updatePreview(PreviewItemType* item, const QPointF& startPoint, const QPointF& currentPoint) {
        item->setLine(QLineF(startPoint, currentPoint));
    }

private:
    /**
     * @brief 内部初始化，添加两个自由句柄
     * @param p1 句柄1的 *本地* 坐标
     * @param p2 句柄2的 *本地* 坐标
     */
    inline void initialize(const QPointF& p1, const QPointF& p2)
    {
        m_h1 = addFreeHandle(p1);
        m_h2 = addFreeHandle(p2);
        m_hMid = addFreeHandle((p1 + p2) / 2.0);
        setFlag(QGraphicsItem::ItemIsMovable, false);
    }
    Handle *m_h1 = nullptr;
    Handle *m_h2 = nullptr;
    Handle *m_hMid = nullptr;
};

#endif // LINESEGMENTROI_H
