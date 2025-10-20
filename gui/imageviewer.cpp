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
    // 配置视图
    // setMouseTracking(true);
    // viewport()->setMouseTracking(true);
    // setDragMode(QGraphicsView::ScrollHandDrag);  // 启用拖动（平移）
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // 缩放锚点为鼠标位置
    setRenderHint(QPainter::SmoothPixmapTransform, true);  // 平滑缩放
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 隐藏滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);  // 无边框
}

void ImageViewer::loadImage(const QString &filePath)
{
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        // 改进的失败日志，提示检查文件和插件
        qDebug() << "Failed to load image at path:" << filePath << ". Check if the file exists and Qt image plugins (e.g., qjpeg.dll, qpng.dll) are properly deployed with the executable."; // <--- MODIFIED
        return;
    }

    qDebug() << "Image loaded successfully:" << filePath << " | Size:" << pixmap.size(); // <--- ADDED success log


    // 清空场景
    m_scene->clear();
    m_pixmapItem = new QGraphicsPixmapItem(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);  // 图片项平滑变换
    m_scene->addItem(m_pixmapItem);
    m_scene->setSceneRect(m_pixmapItem->boundingRect()); // 设置场景范围为图片大小

    // 计算自适应缩放
    fitToView();
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


void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "ImageViewer mouseMoveEvent at:" << event->pos();
    QGraphicsView::mouseMoveEvent(event); // 传递给基类
}
