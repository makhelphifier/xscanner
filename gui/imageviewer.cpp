#include "gui/imageviewer.h"
#include <QDebug>
#include <QPainter>
#include <QScrollBar>

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), m_initialScale(1.0), m_pixmapItem(nullptr)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::HighQualityAntialiasing);
    // 配置视图
    setDragMode(QGraphicsView::ScrollHandDrag);  // 启用拖动（平移）
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // 缩放锚点为鼠标位置
    setRenderHint(QPainter::SmoothPixmapTransform, true);  // 平滑缩放
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 隐藏滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);  // 无边框
}
void ImageViewer::loadImage(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        qDebug() << "Failed to load image at path:" << filePath << ". Check if the file exists and Qt image plugins are properly deployed.";
        setImage(QImage()); // 传递一个空图像来清空视图
        return;
    }
    setImage(image);
}
void ImageViewer::fitToView()
{
    if (!m_pixmapItem) return;

    // 使用更可靠的 fitInView 方法来实现自适应和居中

    // 1. 重置所有变换
    resetTransform();

    // 2. 将整个场景（即图片项的边界）自适应到视图中，并保持长宽比
    // fitInView 内部会自动处理缩放和居中
    fitInView(m_pixmapItem, Qt::KeepAspectRatio);

    // 3. 重新计算并保存初始缩放因子，用于后续的缩放限制。
    // transform().m11() 是当前的 X 轴缩放因子
    m_initialScale = transform().m11();
    emit scaleChanged(1.0); // <-- 添加此行: fit 时比例为 1.0X
}

void ImageViewer::resetView()
{
    fitToView();  // 重置到自适应状态
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier)) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    if (!m_pixmapItem) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    // 计算缩放因子
    qreal factor = (event->angleDelta().y() > 0) ? 1.05 : 0.95;
    scale(factor, factor);



    // 可选：限制缩放范围
    qreal currentScale = transform().m11(); // 当前变换矩阵的 m11 元素即为 x 轴缩放因子
    qreal relativeScale = currentScale / m_initialScale; // 相对初始自适应缩放
    emit scaleChanged(relativeScale);

    // 最小 0.1 倍，最大 10 倍
    if (relativeScale < 0.1) {
        scale(0.1 * m_initialScale / currentScale, 0.1 * m_initialScale / currentScale);
    }
    if (relativeScale > 10.0) {
        scale(10.0 * m_initialScale / currentScale, 10.0 * m_initialScale / currentScale);
    }

    event->accept();
}

void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_R) {  // 按 'R' 重置
        resetView();
    }
    QGraphicsView::keyPressEvent(event);
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // 窗口大小变化时，重新计算自适应缩放并重置
    fitToView();
}

void ImageViewer::setScale(qreal scale)
{
    if (!m_pixmapItem || m_initialScale <= 0) return;

    // 计算目标 m11 值
    qreal targetM11 = scale * m_initialScale;

    // 获取当前 m11 值
    qreal currentM11 = transform().m11();
    if (qFuzzyCompare(currentM11, targetM11)) return; // 如果值相同，则不执行任何操作

    // 重置变换并应用新的缩放
    resetTransform(); // 先回到初始状态
    QGraphicsView::scale(targetM11, targetM11); // 再应用绝对缩放值
}


void ImageViewer::updatePixmap(const QPixmap &pixmap)
{
    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(pixmap);
    }
}


void ImageViewer::setImage(const QImage &image)
{
    if (image.isNull()) {
        // 如果图像为空，则清空场景
        if (m_pixmapItem) m_pixmapItem->setPixmap(QPixmap());
        if (m_borderItem) m_borderItem->setRect(QRectF());
        return;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    qDebug() << "Image set successfully | Size:" << pixmap.size();

    if (!m_pixmapItem) {
        m_pixmapItem = new QGraphicsPixmapItem();
        m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
        m_scene->addItem(m_pixmapItem);
    }
    m_pixmapItem->setPixmap(pixmap);

    if (!m_borderItem) {
        m_borderItem = new QGraphicsRectItem(m_pixmapItem);
        m_borderItem->setPen(QPen(Qt::white, 2));
    }
    m_borderItem->setRect(m_pixmapItem->boundingRect());

    m_scene->setSceneRect(m_pixmapItem->boundingRect());

    fitToView();
}
