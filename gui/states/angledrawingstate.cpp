#include "angledrawingstate.h"
#include "drawingstatemachine.h"
#include "gui/views/imageviewer.h"
#include "gui/items/angleroi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QPen>
#include <QtMath>
#include <QDebug>

AngleDrawingState::AngleDrawingState(DrawingStateMachine* machine, QObject *parent)
    : DrawingState(machine, parent)
{
}

AngleDrawingState::~AngleDrawingState()
{
    // 确保退出时清理
    clearPreviewItems();
}

void AngleDrawingState::enter()
{
    qDebug() << "Entering AngleDrawingState";
    // 重置点击计数器和预览
    m_clickCount = 0;
    clearPreviewItems();
}

void AngleDrawingState::exit()
{
    qDebug() << "Exiting AngleDrawingState";
    // 确保在离开状态时（例如切换工具）清理预览
    clearPreviewItems();
}

bool AngleDrawingState::handleMousePressEvent(QMouseEvent *event)
{
    ImageViewer* viewer = machine()->viewer();
    if (!viewer || event->button() != Qt::LeftButton) {
        return false; // 只响应左键
    }

    QPointF scenePos = viewer->mapToScene(event->pos());
    m_clickCount++;
    qDebug() << "AngleDrawingState: Click" << m_clickCount;

    if (m_clickCount == 1) {
        // --- 点击 1: 定义端点 A ---
        m_pA = scenePos;

        // 创建预览线 (A -> Mouse)
        QPen previewPen(Qt::yellow, 1, Qt::DashLine);
        m_previewLineA = new QGraphicsLineItem(QLineF(m_pA, m_pA));
        m_previewLineA->setPen(previewPen);
        viewer->scene()->addItem(m_previewLineA);

    } else if (m_clickCount == 2) {
        // --- 点击 2: 定义顶点 V ---
        m_pV = scenePos;

        // 1. 固化预览线 A (A -> V)
        m_previewLineA->setLine(QLineF(m_pA, m_pV));

        // 2. 创建预览线 B (V -> Mouse)
        QPen previewPen(Qt::yellow, 1, Qt::DashLine);
        m_previewLineB = new QGraphicsLineItem(QLineF(m_pV, m_pV));
        m_previewLineB->setPen(previewPen);
        viewer->scene()->addItem(m_previewLineB);

        // 3. 创建预览标签
        m_previewLabel = new QGraphicsTextItem();
        m_previewLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        m_previewLabel->setDefaultTextColor(Qt::yellow);
        m_previewLabel->setHtml(R"(<div style='background-color: rgba(0,0,0,150);
                                   color: yellow; padding: 2px;'>0.0°</div>)");
        viewer->scene()->addItem(m_previewLabel);
        updatePreview(scenePos); // 立即更新一次

    } else if (m_clickCount == 3) {
        // --- 点击 3: 定义端点 B ---
        QPointF pB = scenePos;

        // 1. 创建最终的 AngleROI
        AngleROI* angleItem = new AngleROI(m_pA, m_pV, pB);

        // 2. (重要) 将其与 ROI 属性（边界、吸附等）关联
        if (ROI* roiItem = dynamic_cast<ROI*>(angleItem)) {
            roiItem->setMaxBounds(viewer->imageBounds());
            roiItem->setTranslateSnap(1.0); // 1 像素吸附
            connect(roiItem, &ROI::extractRequested,
                    viewer, &ImageViewer::onExtractRegion);
        }

        viewer->scene()->addItem(angleItem);

        // 3. 清理预览项
        clearPreviewItems();

        // 4. 返回 Idle 状态
        machine()->setState(DrawingStateMachine::Idle);
    }

    event->accept();
    return true; // 事件已处理
}

bool AngleDrawingState::handleMouseMoveEvent(QMouseEvent *event)
{
    if (m_clickCount == 0) {
        return false; // 还没开始
    }

    QPointF scenePos = machine()->viewer()->mapToScene(event->pos());

    if (m_clickCount == 1 && m_previewLineA) {
        // 移动 (A -> Mouse)
        m_previewLineA->setLine(QLineF(m_pA, scenePos));
    } else if (m_clickCount == 2 && m_previewLineB) {
        // 移动 (V -> Mouse)，并更新角度
        updatePreview(scenePos);
    }

    event->accept();
    return true;
}

bool AngleDrawingState::handleMouseReleaseEvent(QMouseEvent *event)
{
    // 在这个状态机中，释放事件不做任何事
    return true;
}

bool AngleDrawingState::handleWheelEvent(QWheelEvent *event)
{
    // 绘制时忽略滚轮
    qDebug() << "AngleDrawingState: Ignoring wheel event.";
    return true;
}

void AngleDrawingState::clearPreviewItems()
{
    QGraphicsScene* scene = machine()->viewer()->scene();
    if (!scene) return;

    if (m_previewLineA) {
        scene->removeItem(m_previewLineA);
        delete m_previewLineA;
        m_previewLineA = nullptr;
    }
    if (m_previewLineB) {
        scene->removeItem(m_previewLineB);
        delete m_previewLineB;
        m_previewLineB = nullptr;
    }
    if (m_previewLabel) {
        scene->removeItem(m_previewLabel);
        delete m_previewLabel;
        m_previewLabel = nullptr;
    }
}

void AngleDrawingState::updatePreview(const QPointF& currentMousePos)
{
    if (!m_previewLineB || !m_previewLabel) return;

    // 1. 更新预览线 B
    m_previewLineB->setLine(QLineF(m_pV, currentMousePos));

    // 2. 计算角度
    QLineF lineVA(m_pV, m_pA);
    QLineF lineVB(m_pV, currentMousePos);

    qreal angle = 0.0;
    if (lineVA.length() > 0 && lineVB.length() > 0) {
        angle = lineVB.angleTo(lineVA);
        if (angle > 180.0) {
            angle = 360.0 - angle;
        }
    }

    // 3. 更新标签
    QString text = QString::number(angle, 'f', 1) + "°";
    m_previewLabel->setHtml(QString(R"(<div style='background-color: rgba(0,0,0,150);
                                   color: yellow; padding: 2px;'>%1</div>)").arg(text));

    // 4. 定位标签
    m_previewLabel->setPos(m_pV + QPointF(10, 10)); // 简单放在 V 点右下方
}
