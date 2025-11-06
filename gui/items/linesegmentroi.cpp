#include "linesegmentroi.h"
#include "handle.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QDebug>
#include <QPen>
#include <QPointF>
#include <QLineF>
#include <QApplication>
#include <QTransform>


void LineSegmentROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_handles.size() < 2) return;

    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen p = m_currentPen;
    p.setWidth(2);
    painter->setPen(p);

    // 句柄的 'pos' 存储的是本地坐标
    QPointF h1 = m_handles[0].pos;
    QPointF h2 = m_handles[1].pos;

    painter->drawLine(h1, h2);
}

QRectF LineSegmentROI::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath LineSegmentROI::shape() const
{
    QPainterPath path;
    if (m_handles.size() < 2) return path;

    QPointF h1 = m_handles[0].pos;
    QPointF h2 = m_handles[1].pos;

    QLineF line(h1, h2);
    if (line.length() == 0) {
        if (m_handles[0].item) path.addRect(m_handles[0].item->boundingRect().translated(h1));
        return path;
    }

    QPainterPathStroker stroker;
    stroker.setWidth(10.0);
    stroker.setCapStyle(Qt::FlatCap);
    path.moveTo(h1);
    path.lineTo(h2);
    QPainterPath lineShape = stroker.createStroke(path);

    for (const auto& handleInfo : qAsConst(m_handles)) {
        if(handleInfo.item) {
            lineShape.addRect(handleInfo.item->boundingRect().translated(handleInfo.pos));
        }
    }

    return lineShape;
}
void LineSegmentROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    bool snap = (QApplication::keyboardModifiers() & Qt::ControlModifier);
    QPointF clampedScenePos = scenePos;
    if (m_maxBounds.isValid()) { // 使用 protected m_maxBounds
        QRectF bounds = m_maxBounds.toRectF();
        clampedScenePos.setX(qMax(bounds.left(), qMin(scenePos.x(), bounds.right())));
        clampedScenePos.setY(qMax(bounds.top(), qMin(scenePos.y(), bounds.bottom())));
    }

    ROIState newState = m_preMoveState;
    QPointF p1_local = clampedScenePos;
    QTransform preMove_LocalToScene_Transform;
    preMove_LocalToScene_Transform.translate(newState.pos.x(), newState.pos.y());

    bool invertible = false;
    QTransform preMove_SceneToLocal_Transform = preMove_LocalToScene_Transform.inverted(&invertible);
    if (invertible) {
        p1_local = preMove_SceneToLocal_Transform.map(clampedScenePos);
    } else {
        p1_local = clampedScenePos;
    }

    int handleIndex = indexOfHandle(handle);
    if (handleIndex < 0) return;

    HandleInfo& info = m_handles[handleIndex];

    if (handle == m_hMid) {
        QPointF delta = p1_local - info.pos;

        if (snap && m_translateSnapSize > 0) {
            delta.setX(snapValue(delta.x(), m_translateSnapSize));
            delta.setY(snapValue(delta.y(), m_translateSnapSize));
            if (delta.isNull()) {
                stateChanged(finish);
                return;
            }
        }
        m_handles[0].pos += delta;
        m_handles[1].pos += delta;
        m_handles[2].pos += delta;

    } else if (handle == m_h1 || handle == m_h2) {
        if (snap && m_translateSnapSize > 0) {
            p1_local.setX(snapValue(p1_local.x(), m_translateSnapSize));
            p1_local.setY(snapValue(p1_local.y(), m_translateSnapSize));
        }
        info.pos = p1_local;
        m_handles[2].pos = (m_handles[0].pos + m_handles[1].pos) / 2.0;
    }
    stateChanged(finish);
}
