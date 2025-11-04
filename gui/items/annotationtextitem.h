#ifndef ANNOTATIONTEXTITEM_H
#define ANNOTATIONTEXTITEM_H

#include <QGraphicsTextItem>

/**
 * @brief 一个可交互、可编辑的文本项。
 *
 * - 可通过拖动来移动。
 * - 缩放视图时，字体大小保持不变 (ItemIgnoresTransformations)。
 * - 双击可进入编辑模式。
 * - 点击外部可退出编辑模式。
 */
class AnnotationTextItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    explicit AnnotationTextItem(const QPointF& pos, QGraphicsItem* parent = nullptr);
    virtual ~AnnotationTextItem() override = default;

    enum { Type = UserType + 11 }; // FreehandROI 是 +10
    int type() const override { return Type; }

protected:
    /**
     * @brief [重写] 当失去焦点时（例如点击别处），退出编辑模式。
     */
    void focusOutEvent(QFocusEvent *event) override;

    /**
     * @brief [重写] 当双击时，进入编辑模式。
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // ANNOTATIONTEXTITEM_H
