#include "gui/items/angledlineroi.h"
#include "handle.h"
#include <QApplication>
#include <QTransform>
#include <QMenu>
#include <QtMath>
#include <QDebug>


void AngledLineROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    bool snap = (QApplication::keyboardModifiers() & Qt::ControlModifier);
    QPointF clampedScenePos = scenePos;
    if (m_maxBounds.isValid()) {
        QRectF bounds = m_maxBounds.toRectF();
        clampedScenePos.setX(qMax(bounds.left(), qMin(scenePos.x(), bounds.right())));
        clampedScenePos.setY(qMax(bounds.top(), qMin(scenePos.y(), bounds.bottom())));
    }

    QPointF p1_local = clampedScenePos;

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
        // 更新中点
        m_handles[2].pos = (m_handles[0].pos + m_handles[1].pos) / 2.0;
    }

    prepareGeometryChange();
    stateChanged(finish);
}


void AngledLineROI::setViewer(ImageViewer* viewer)
{
    m_viewer = viewer;
    if (m_viewer) {
        connect(m_viewer, &ImageViewer::viewZoomed, this, &AngledLineROI::onViewChanged);
        connect(m_viewer, &ImageViewer::viewPanned, this, &AngledLineROI::onViewChanged);
        // 立即更新一次
        QTimer::singleShot(0, this, &AngledLineROI::onViewChanged);
    }
}

void AngledLineROI::onViewChanged()
{
    if (m_cachedBoundingRect.isValid()) {
        prepareGeometryChange();
        m_cachedBoundingRect = QRectF();
    }
}

QRectF AngledLineROI::boundingRect() const
{
    if (m_cachedBoundingRect.isValid()) {
        return m_cachedBoundingRect;
    }
    if (!m_viewer || m_handles.size() < 2) {
        return QRectF(-1000, -5, 2000, 10); // 默认
    }

    QRectF sceneViewRect = m_viewer->mapToScene(m_viewer->viewport()->rect()).boundingRect();
    QLineF line(m_handles[0].pos, m_handles[1].pos);
    if (qFuzzyCompare(line.length(), 0)) {
        return m_handles[0].item->boundingRect().translated(m_handles[0].pos);
    }
    QTransform t;
    t.translate(line.center().x(), line.center().y());
    t.rotate(-line.angle());
    QRectF localViewRect = t.inverted().map(sceneViewRect).boundingRect();
    qreal padding = 10.0;
    QRectF infiniteRect(localViewRect.left() - padding,
                        -padding,
                        localViewRect.width() + padding*2,
                        padding*2);
    m_cachedBoundingRect = t.map(infiniteRect).boundingRect();
    return m_cachedBoundingRect;
}

void AngledLineROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_handles.size() < 2) return;
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen p = m_currentPen;
    p.setWidth(2);
    painter->setPen(p);
    QLineF line(m_handles[0].pos, m_handles[1].pos);
    if (qFuzzyCompare(line.length(), 0)) return;
    QRectF rect = boundingRect();
    QLineF drawLine = line.unitVector();
    drawLine.setP1(line.pointAt(-rect.width() - rect.height()));
    drawLine.setP2(line.pointAt(rect.width() + rect.height()));
    painter->drawLine(drawLine);
}


QPainterPath AngledLineROI::shape() const
{
    // shape 逻辑复制自 LineSegmentROI，用于点击检测
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
    stroker.setWidth(10.0); // 10px 的点击宽度
    stroker.setCapStyle(Qt::FlatCap);
    path.moveTo(h1);
    path.lineTo(h2);
    QPainterPath lineShape = stroker.createStroke(path);

    // 添加手柄的 shape
    for (const auto& handleInfo : qAsConst(m_handles)) {
        if(handleInfo.item) {
            lineShape.addPath(handleInfo.item->mapToParent(handleInfo.item->shape()));
        }
    }

    return lineShape;
}

qreal AngledLineROI::getLineAngle() const
{
    if (m_handles.size() < 2) return 0.0;

    QLineF line(m_handles[0].pos, m_handles[1].pos);
    qreal angle = line.angle(); // 0-360, 0=向右

    // 归一化到 -180 到 180
    if (angle > 180.0) {
        angle -= 360.0;
    }
    return angle;
}

void AngledLineROI::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    qreal angleH = getLineAngle(); // -180 到 180

    // 计算相对于 (0-90) 的水平夹角
    qreal acuteAngleH = angleH;
    if (acuteAngleH > 90.0) acuteAngleH = 180.0 - acuteAngleH;
    else if (acuteAngleH < -90.0) acuteAngleH = -180.0 - acuteAngleH;

    qreal acuteAngleV = 90.0 - acuteAngleH;
    if (acuteAngleV > 90.0) acuteAngleV = 180.0 - acuteAngleV;
    else if (acuteAngleV < -90.0) acuteAngleV = -180.0 - acuteAngleV;


    QMenu menu;

    // 添加角度信息 (设置为 disabled，使其像标签一样)
    QString hText = QString("与水平夹角: %1°").arg(qAbs(acuteAngleH), 0, 'f', 1);
    QString vText = QString("与垂直夹角: %1°").arg(qAbs(acuteAngleV), 0, 'f', 1);

    menu.addAction(hText)->setEnabled(false);
    menu.addAction(vText)->setEnabled(false);
    menu.addSeparator();

    menu.exec(event->screenPos());
}
