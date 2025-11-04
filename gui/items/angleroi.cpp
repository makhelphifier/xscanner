#include "angleroi.h"
#include "handle.h"
#include <QPainter>
#include <QPen>
#include <QMenu>
#include <QtMath>
#include <QDebug>

AngleROI::AngleROI(const QPointF& pA, const QPointF& pV, const QPointF& pB, QGraphicsItem* parent)
    : ROI(QPointF(0,0), QSizeF(1,1), parent)// 基类 pos/size 无意义
{
    // 1. 创建手柄 (FreeHandle 不受 ROI 的 pos/size 影响)
    // 注意：我们使用 pV 作为 ROI 的基类 pos，但意义不大
    // setPos(pV);
    m_hA = addFreeHandle(pA);
    m_hV = addFreeHandle(pV);
    m_hB = addFreeHandle(pB);

    // 2. 创建视觉子项
    QPen linePen(Qt::yellow, 2, Qt::SolidLine);
    m_lineVA = new QGraphicsLineItem(this);
    m_lineVA->setPen(linePen);
    m_lineVB = new QGraphicsLineItem(this);
    m_lineVB->setPen(linePen);

    m_label = new QGraphicsTextItem(this);
    m_label->setFlag(QGraphicsItem::ItemIgnoresTransformations); // 文本不随缩放
    m_label->setDefaultTextColor(Qt::yellow);
    m_label->setHtml(R"(<div style='background-color: rgba(0,0,0,150);
                           color: yellow; padding: 2px;'></div>)");

    // 3. 初始计算
    updateAngle();

    setFlag(QGraphicsItem::ItemIsMovable, false); // 不允许拖动ROI本体
}

void AngleROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // 基类 ROI::paint() 会画一个矩形，我们不希望那样
    // 所以这个函数重写后保持为空。
    // 所有的绘制都由子项 m_lineVA, m_lineVB, m_label 完成。

    // (可选) 绘制圆弧
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::yellow, 1, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);

    if (!m_hA || !m_hV || !m_hB) return;

    QLineF lineVA(m_hV->pos(), m_hA->pos());
    QLineF lineVB(m_hV->pos(), m_hB->pos());

    if (lineVA.length() == 0 || lineVB.length() == 0) return;

    // 绘制圆弧
    qreal radius = qMin(lineVA.length(), lineVB.length()) * 0.3; // 30% 半径
    if (radius > 50) radius = 50; // 最大 50px
    if (radius < 10) radius = 10; // 最小 10px

    QRectF arcRect(m_hV->pos() - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    qreal startAngle = lineVB.angle(); // QLineF 角度
    qreal sweepAngle = lineVA.angle() - startAngle;

    // 确保 sweepAngle 是内角
    if (sweepAngle > 180) sweepAngle -= 360;
    if (sweepAngle < -180) sweepAngle += 360;

    painter->drawArc(arcRect, static_cast<int>(startAngle * 16), static_cast<int>(sweepAngle * 16));
}

QRectF AngleROI::boundingRect() const
{
    // 计算一个能包围所有手柄和标签的矩形
    if (!m_hA || !m_hV || !m_hB || !m_label) return QRectF();

    QPainterPath path;
    path.addRect(m_hA->mapToParent(m_hA->boundingRect()).boundingRect());
    path.addRect(m_hV->mapToParent(m_hV->boundingRect()).boundingRect());
    path.addRect(m_hB->mapToParent(m_hB->boundingRect()).boundingRect());
    path.addRect(m_lineVA->boundingRect());
    path.addRect(m_lineVB->boundingRect());
    path.addRect(m_label->mapToParent(m_label->boundingRect()).boundingRect());

    return path.boundingRect();
}

QPainterPath AngleROI::shape() const
{
    // 让点击检测只落在手柄和线上
    QPainterPath path;
    if (!m_hA || !m_hV || !m_hB || !m_lineVA || !m_lineVB) return path;

    path.addPath(m_hA->mapToParent(m_hA->shape()));
    path.addPath(m_hV->mapToParent(m_hV->shape()));
    path.addPath(m_hB->mapToParent(m_hB->shape()));

    // 为线条创建更宽的 shape
    QPainterPathStroker stroker;
    stroker.setWidth(10.0); // 10px 点击宽度

    QPainterPath linePathA;
    linePathA.moveTo(m_lineVA->line().p1());
    linePathA.lineTo(m_lineVA->line().p2());
    path.addPath(stroker.createStroke(linePathA));

    QPainterPath linePathB;
    linePathB.moveTo(m_lineVB->line().p1());
    linePathB.lineTo(m_lineVB->line().p2());
    path.addPath(stroker.createStroke(linePathB));

    return path;
}


void AngleROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    // 直接调用基类的 FreeHandle 逻辑
    ROI::movePoint(handle, scenePos, finish);

    // 更新视觉效果
    if (m_hA && m_hV && m_hB) {
        updateAngle();
    }
}

void AngleROI::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;

    // 添加角度信息
    QString angleText = QString("角度: %1°").arg(m_currentAngle, 0, 'f', 1);
    menu.addAction(angleText)->setEnabled(false);
    menu.addSeparator();

    // 添加 "提取" (尽管对角度测量意义不大，但保持一致性)
    QAction* extractAction = menu.addAction("提取区域 (Extract Region)");
    connect(extractAction, &QAction::triggered, this, [this]() {
        emit extractRequested(this);
    });

    menu.exec(event->screenPos());
}

void AngleROI::updateAngle()
{
    prepareGeometryChange(); // 通知 boundingRect 即将改变

    QPointF pA = m_hA->pos();
    QPointF pV = m_hV->pos();
    QPointF pB = m_hB->pos();

    // 1. 更新线
    m_lineVA->setLine(QLineF(pV, pA));
    m_lineVB->setLine(QLineF(pV, pB));

    // 2. 计算角度
    QLineF lineVA(pV, pA);
    QLineF lineVB(pV, pB);

    qreal angle = 0.0;
    if (lineVA.length() > 0 && lineVB.length() > 0) {
        angle = lineVB.angleTo(lineVA);
        // 始终取内角
        if (angle > 180.0) {
            angle = 360.0 - angle;
        }
    }
    m_currentAngle = angle;

    // 3. 更新标签
    QString text = QString::number(m_currentAngle, 'f', 1) + "°";
    m_label->setHtml(QString(R"(<div style='background-color: rgba(0,0,0,150);
                               color: yellow; padding: 2px;'>%1</div>)").arg(text));

    // 4. 定位标签
    // 将标签放在V点附近，但要避开线
    qreal offset = 20.0; // 偏移 20 像素
    QLineF bisector = lineVB;
    bisector.setAngle(lineVB.angle() + (lineVA.angle() - lineVB.angle()) / 2.0);
    QLineF labelPosLine(pV, pV + QPointF(1,0));
    labelPosLine.setAngle(bisector.angle());
    labelPosLine.setLength(offset);

    m_label->setPos(labelPosLine.p2());

    update(); // 请求重绘
}
