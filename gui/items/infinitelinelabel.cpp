#include "infinitelinelabel.h"
#include "infinitelineitem.h"
#include "views/imageviewer.h"
#include <QDebug>
#include <QGraphicsItem>

InfiniteLineLabel::InfiniteLineLabel(InfiniteLineItem* parent)
    : QGraphicsTextItem(parent), m_line(parent)
{
    if (!m_line) {
        qWarning() << "InfiniteLineLabel created without a valid InfiniteLineItem parent.";
        return;
    }

    // 设置默认样式
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setDefaultTextColor(Qt::white);
    setHtml(R"(<div style='background-color: rgba(0,0,0,150);
                           color: white;
                           padding: 2px;
                           border-radius: 2px;'>
                  </div>)");

    // 根据父项的当前角度设置初始格式
    setFormat(m_line->angle());
    // 设置初始文本
    updateText(m_line->value());

    // 1. 当线条位置改变时，更新文本
    connect(m_line, &InfiniteLineItem::positionChanged, this, &InfiniteLineLabel::updateText);
}

void InfiniteLineLabel::setFormat(qreal angle)
{
    // 根据角度设置格式化字符串
    if (angle == 90) {
        m_format = "X: %1";
    } else {
        m_format = "Y: %1";
    }
}

void InfiniteLineLabel::updateText(qreal value)
{
    // 使用格式化字符串更新 HTML 内容
    QString text = m_format.arg(value, 0, 'f', 1); // 格式化为1位小数
    setHtml(QString(R"(<div style='background-color: rgba(0,0,0,150);
                               color: white;
                               padding: 2px;
                               border-radius: 2px;'>
                      %1
                      </div>)").arg(text));
}

void InfiniteLineLabel::onViewChanged()
{
    if (!m_line) return;
    ImageViewer* viewer = m_line->viewer();
    if (!viewer) return;

    // 1. 获取视图在场景中的矩形
    QRectF sceneViewRect = viewer->mapToScene(viewer->viewport()->rect()).boundingRect();

    // 2. 将场景矩形转换为线条的局部坐标系
    //    (线条的局部坐标系中，线条总是在 Y=0 的水平线上)
    QRectF localViewRect = m_line->mapFromScene(sceneViewRect).boundingRect();

    // 3. 将标签定位在局部可见区域的左侧，并偏离线条一点
    qreal xPos = localViewRect.left() + 10; // 距离左边缘10像素
    qreal yPos = 10;                        // 距离线条10像素 (局部Y轴)

    // 4. 设置标签在父项（线条）坐标系中的位置
    setPos(xPos, yPos);
}
