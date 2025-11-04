#ifndef ELLIPSEROI_H
#define ELLIPSEROI_H

#include "roi.h"
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QApplication> // 用于检查 Ctrl 键
#include <QtMath>       // 用于 qAbs, qSign

/**
 * @brief 一个预设了缩放句柄的椭圆形ROI子类。
 *
 * 继承自ROI，但在构造时自动添加了常用的Scale/Rotate Handles。
 * 它适配了 GenericDrawingState，因此可以被交互式地绘制。
 */
class EllipseROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数1: 用于代码直接创建 (添加句柄)
    explicit EllipseROI(const QPointF& pos, const QSizeF& size, QGraphicsItem* parent = nullptr);

    // 构造函数2: 用于 GenericDrawingState 从起点和终点创建
    explicit EllipseROI(const QPointF& startPoint, const QPointF& endPoint, QGraphicsItem* parent = nullptr);

    virtual ~EllipseROI() override = default;

    enum { Type = UserType + 12 }; // 假设 TextItem 是 +11
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;


    // --- 为 GenericDrawingState 提供的适配接口 ---

    // 1. 定义预览时使用的图形项类型
    using PreviewItemType = QGraphicsEllipseItem;

    // 2. 定义如何初始化预览项的静态方法
    static void initPreview(PreviewItemType* item) {
        QPen pen(Qt::yellow, 1, Qt::DashLine);
        item->setPen(pen);
        item->setBrush(Qt::transparent);
        item->setZValue(1000); // 确保预览在最上层
    }

    // 3. 定义如何根据起点和当前点更新预览的静态方法
    static void updatePreview(PreviewItemType* item, const QPointF& startPoint, const QPointF& currentPoint) {
        // [Ctrl 键约束逻辑]
        QRectF rect = constrainRectToSquare(startPoint, currentPoint);
        item->setRect(rect.normalized());
    }

private:
    /**
     * @brief [新增] 检查Ctrl键，将矩形约束为正方形
     */
    static QRectF constrainRectToSquare(const QPointF& p1, const QPointF& p2)
    {
        if (!(QApplication::keyboardModifiers() & Qt::ControlModifier)) {
            // 未按下 Ctrl，返回普通矩形
            return QRectF(p1, p2);
        }

        // 按下 Ctrl，约束为正方形
        qreal dx = p2.x() - p1.x();
        qreal dy = p2.y() - p1.y();

        // 以 p1 为锚点，取 dx 和 dy 中绝对值较大者作为边长
        qreal maxDim = qMax(qAbs(dx), qAbs(dy));

        // 使用 qSign 保持正确的拖动方向（例如拖向左上角）
        QPointF constrainedEnd(p1.x() + maxDim * qSin(dx),
                               p1.y() + maxDim * qSin(dy));

        return QRectF(p1, constrainedEnd);
    }
};

#endif // ELLIPSEROI_H
