#ifndef INFINITELINEITEM_H
#define INFINITELINEITEM_H

#include <QGraphicsObject>
#include <QPen>

class ImageViewer; // 前向声明
class QPainter;
class QStyleOptionGraphicsItem;

/**
 * @brief 一个可交互的无限长线条item。
 *
 * 它通过监听 ImageViewer 的视口变化信号来动态更新自己的包围盒(boundingRect)，
 * 从而实现在视图中“无限”延伸的效果。
 */
class InfiniteLineItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param viewer 指向承载此 item 的 ImageViewer，用于获取视口变化。
     * @param angle 初始角度 (0=水平, 90=垂直)。
     * @param parent 父 QGraphicsItem。
     */
    explicit InfiniteLineItem(ImageViewer* viewer, qreal angle = 90, QGraphicsItem* parent = nullptr);
    virtual ~InfiniteLineItem() = default;

    // --- QGraphicsItem 重写 ---
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // --- 公共接口 ---
    void setAngle(qreal angle);
    qreal angle() const { return m_angle; }

    void setMovable(bool movable) { m_movable = movable; }
    bool isMovable() const { return m_movable; }

    void setPen(const QPen& pen);
    void setHoverPen(const QPen& pen);

    // 设置/获取线条的位置 (垂直线为X值, 水平线为Y值)
    void setValue(qreal value);
    qreal value() const;

signals:
    /**
     * @brief 当线条位置在拖动过程中发生变化时发射。
     * @param value 新的位置值 (x 或 y)。
     */
    void positionChanged(qreal value);

    /**
     * @brief 当拖动完成时发射。
     * @param value 最终的位置值 (x 或 y)。
     */
    void positionChangeFinished(qreal value);

protected:
    // --- 事件处理 ---
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private slots:
    /**
     * @brief 响应视图变化（平移或缩放），清除缓存的包围盒。
     */
    void onViewChanged();

private:
    /**
     * @brief 强制重新计算包围盒。
     * @return 当前视口对应的包围盒 (在 item 局部坐标系中)。
     */
    QRectF calculateBoundingRect() const;

    ImageViewer* m_viewer; // 指向视图
    qreal m_angle;
    bool m_movable = true;
    bool m_isMoving = false;

    QPen m_pen;
    QPen m_hoverPen;
    QPen m_currentPen;

    // 缓存包围盒，用于性能优化
    mutable QRectF m_cachedBoundingRect;
};

#endif // INFINITELINEITEM_H
