#ifndef ANGLEROI_H
#define ANGLEROI_H

#include "roi.h"
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPainterPath>

/**
 * @brief 一个用于测量角度的 ROI。
 *
 * 由三个自由手柄（A, V, B）定义，V 是顶点。
 * 它会绘制两条线段 (V-A, V-B) 和一个显示角度的文本标签。
 */
class AngleROI : public ROI
{
    Q_OBJECT

public:
    // 构造函数: 由三个点定义
    AngleROI(const QPointF& pA, const QPointF& pV, const QPointF& pB, QGraphicsItem* parent = nullptr);
    virtual ~AngleROI() override = default;

    enum { Type = UserType + 7 }; // +6 已被 AngledLineROI 使用
    int type() const override { return Type; }

    // --- QGraphicsItem 重写 ---
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // --- ROI 重写 ---
    // 当手柄被拖动时
    void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true) override;

protected:
    // 重写右键菜单（可以继承ROI的，也可以自定义）
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    /**
     * @brief 根据手柄位置更新线段和角度文本
     */
    void updateAngle();

    Handle *m_hA; // 端点 A
    Handle *m_hV; // 顶点 V
    Handle *m_hB; // 端点 B

    // 子项，用于显示
    QGraphicsLineItem* m_lineVA;
    QGraphicsLineItem* m_lineVB;
    QGraphicsTextItem* m_label;

    qreal m_currentAngle = 0.0;
};

#endif // ANGLEROI_H
