#ifndef POINTMEASUREITEM_H
#define POINTMEASUREITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include <QFontMetrics>

class ImageViewModel;
/**
 * @brief 一个可交互的点测量项。
 *
 * 它显示一个十字标记和 (X, Y, Value) 文本。
 * 它使用 ItemIgnoresTransformations 来保持恒定的像素大小。
 * 它可以被拖动，并在拖动结束后更新其值。
 */
class PointMeasureItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param pos 初始的场景坐标
     * @param viewer 指向 ImageViewer，用于获取像素值
     * @param parent 父项
     */
    explicit PointMeasureItem(const QPointF& pos, ImageViewModel* viewModel, QGraphicsItem* parent = nullptr);
    virtual ~PointMeasureItem() override = default;

    enum { Type = UserType + 8 }; // +7 已被 AngleROI 使用
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    // --- 事件重写 ---
    /**
     * @brief 拖动释放后更新 (X, Y, Value)
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    /**
     * @brief 根据给定位置更新文本
     */
    void updateTextAndPos(const QPointF& scenePos);

    /**
     * @brief [新] 动态计算文本的边界框（在局部坐标系中）
     */
    QRectF calculateTextRect() const;

    ImageViewModel* m_viewModel;
    QString m_text;        // 缓存要绘制的文本
    QPointF m_textOffset;  // 文本相对于(0,0)十字中心的偏移

    static const int MARKER_SIZE = 5; // 十字标记的半径 (像素)
};

#endif // POINTMEASUREITEM_H
