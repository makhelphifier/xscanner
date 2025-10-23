// In file: gui/annotationhorizontallineitem.h

#include <QGraphicsObject>

class QGraphicsSimpleTextItem;

class AnnotationHorizontalLineItem : public QGraphicsObject
{
    Q_OBJECT

public:
    // 构造函数增加 initialScale 参数
    AnnotationHorizontalLineItem(qreal y, qreal sceneWidth, qreal initialScale, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // 用于更新缩放比例的方法
    void updateScale(qreal scale);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    qreal m_sceneWidth;
    QGraphicsSimpleTextItem *m_label;

    // 用于动态计算字体大小的成员
    qreal m_baseFontSize;
    qreal m_currentScale;
};
