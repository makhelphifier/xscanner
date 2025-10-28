// handle.h

#ifndef HANDLE_H
#define HANDLE_H

#include <QGraphicsObject>
#include <QPen>
#include <QPainterPath>
#include <QVector>
#include <QMap>
#include <QPair> // 用于 QPair
// 前向声明
class ROI;
class QGraphicsSceneMouseEvent;
// 定义句柄类型枚举
enum class HandleType {
    Scale,
    Rotate,
    ScaleRotate,
    Translate, // 可能需要
    Free       // 可能需要
};
/**
 * @brief 可拖动的句柄项，用于控制ROI。
 *
 * Handle是一个独立的QGraphicsObject，它可以被一个或多个ROI拥有。
 * 当它被拖动时，它会通知所有与之关联的ROI。
 */
class Handle : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Handle(HandleType type, QGraphicsItem* parent = nullptr);
    virtual ~Handle() override = default;

    // --- 公共接口 ---
    void connectROI(ROI* roi);
    void disconnectROI(ROI* roi);
    const QList<ROI*>& rois() const; // ++ 新增 getter ++
    void setPen(const QPen& pen);
    void setHoverPen(const QPen& pen);
    enum { Type = UserType + 1 };
    int type() const override { return Type; }
    HandleType handleType() const { return m_type; }

    // --- QGraphicsItem 重写 ---
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setPosInROI(const QPointF& relativePos, const QSizeF& roiSize);
protected:
    // --- 事件处理 ---
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void buildPath(); // 根据类型构建句柄的形状路径

    QList<ROI*> m_rois;      // 关联的ROI列表
    HandleType m_type;
    qreal m_radius = 5.0;    // 句柄的屏幕像素半径

    QPen m_pen;
    QPen m_hoverPen;
    QPen m_currentPen;

    bool m_isMoving = false;
    QPainterPath m_path;     // 句柄的几何形状


};

#endif // HANDLE_H
