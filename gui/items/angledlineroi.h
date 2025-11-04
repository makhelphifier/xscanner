#ifndef ANGLEDLINEITEM_H
#define ANGLEDLINEITEM_H

#include "roi.h"
#include <QGraphicsLineItem> // 用于预览
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include "gui/views/imageviewer.h" // 需要访问 viewer

class ImageViewer; // 前向声明

/**
 * @brief 一个可交互的、无限长的倾斜直线。
 *
 * 它使用 LineSegmentROI 的三个自由句柄（h1, h2, hMid）进行交互，
 * 但使用 InfiniteLineItem 的逻辑来绘制一条无限延伸的线。
 */
class AngledLineROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数: 用于 GenericDrawingState (点在场景坐标系中)
    inline explicit AngledLineROI(const QPointF& startPoint, const QPointF& endPoint, QGraphicsItem* parent = nullptr)
        : ROI(QPointF(0,0), QSizeF(1,1), parent) // 基类 pos/size 无意义
    {
        // 现在直接在头文件中调用 initialize
        initialize(startPoint, endPoint);
    }

    virtual ~AngledLineROI() override = default;

    enum { Type = UserType + 6 }; // +5 已被 InfiniteLineItem 使用
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // --- ROI 重写 ---
    void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true) override;

    // --- 新增公共方法 ---
    /**
     * @brief 必须调用此方法，将ROI与Viewer关联以实现无限延伸
     */
    void setViewer(ImageViewer* viewer);
    ImageViewer* viewer() const { return m_viewer; }

    /**
     * @brief 获取当前直线的角度 (0-360, 0=向右)
     */
    qreal getLineAngle() const;

    // --- 适配 GenericDrawingState ---
    using PreviewItemType = QGraphicsLineItem;

    static void initPreview(PreviewItemType* item) {
        QPen pen(Qt::yellow, 1, Qt::DashLine);
        item->setPen(pen);
        item->setZValue(1000);
    }

    static void updatePreview(PreviewItemType* item, const QPointF& startPoint, const QPointF& currentPoint) {
        item->setLine(QLineF(startPoint, currentPoint));
    }

protected:
    // 重写右键菜单以显示角度
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private slots:
    /**
     * @brief 响应视图变化（平移或缩放），清除缓存的包围盒。
     */
    void onViewChanged();

private:
    /**
     * @brief 内部初始化，添加两个自由句柄和一个中点句柄
     */
    inline void initialize(const QPointF& p1, const QPointF& p2)
    {
        m_h1 = addFreeHandle(p1);
        m_h2 = addFreeHandle(p2);
        m_hMid = addFreeHandle((p1 + p2) / 2.0);
        setFlag(QGraphicsItem::ItemIsMovable, false); // 不允许拖动ROI本体
    }

    Handle *m_h1 = nullptr;
    Handle *m_h2 = nullptr;
    Handle *m_hMid = nullptr;

    ImageViewer* m_viewer = nullptr; // 指向视图
    mutable QRectF m_cachedBoundingRect;
};

#endif // ANGLEDLINEITEM_H
