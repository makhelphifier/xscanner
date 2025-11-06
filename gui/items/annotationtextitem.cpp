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
    setPos(pos);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setDefaultTextColor(Qt::yellow);
    setFont(QFont("Arial", 10));
    setPlainText("在此处键入...");
    setTextInteractionFlags(Qt::NoTextInteraction);
}

/**
 * @brief [重写] 当失去焦点时（例如点击别处），退出编辑模式。
 */
void AnnotationTextItem::focusOutEvent(QFocusEvent *event)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
    if (toPlainText().isEmpty()) {
        delete this;
        return;
    }
    QGraphicsTextItem::focusOutEvent(event);
}

/**
 * @brief [重写] 当双击时，进入编辑模式。
 */
void AnnotationTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (textInteractionFlags() == Qt::NoTextInteraction) {
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setFocus(Qt::MouseFocusReason);
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::Document);
        setTextCursor(cursor);
    }

    QGraphicsTextItem::mouseDoubleClickEvent(event);
}
