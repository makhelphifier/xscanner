#include "annotationtextitem.h"
#include <QFocusEvent>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QFont>
#include <QPen>
#include <QTextCursor>

AnnotationTextItem::AnnotationTextItem(const QPointF& pos, QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    // 1. 设置位置
    setPos(pos);

    // 2. [关键] 设置标志
    //    使其可移动
    setFlag(QGraphicsItem::ItemIsMovable, true);
    //    使其可被选中
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //    [关键] 使其在视图缩放时保持大小不变
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    // 3. 设置默认外观
    setDefaultTextColor(Qt::yellow);
    setFont(QFont("Arial", 10));
    setPlainText("在此处键入...");

    // 4. 默认设置为不可编辑（等待双击）
    setTextInteractionFlags(Qt::NoTextInteraction);
}

/**
 * @brief [重写] 当失去焦点时（例如点击别处），退出编辑模式。
 */
void AnnotationTextItem::focusOutEvent(QFocusEvent *event)
{
    // 设置为不可交互（即退出编辑模式）
    setTextInteractionFlags(Qt::NoTextInteraction);

    // （可选）如果退出时文本为空，则删除自己
    if (toPlainText().isEmpty()) {
        delete this;
        return; // 必须立即返回，因为 'this' 已被删除
    }

    QGraphicsTextItem::focusOutEvent(event);
}

/**
 * @brief [重写] 当双击时，进入编辑模式。
 */
void AnnotationTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // 检查是否当前*未*处于编辑模式
    if (textInteractionFlags() == Qt::NoTextInteraction) {
        // 1. 切换到可编辑模式
        setTextInteractionFlags(Qt::TextEditorInteraction);
        // 2. 立即获取焦点，以便用户可以打字
        setFocus(Qt::MouseFocusReason);
        // 3. （可选）全选文本
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::Document);
        setTextCursor(cursor);
    }

    QGraphicsTextItem::mouseDoubleClickEvent(event);
}
