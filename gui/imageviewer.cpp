#include "gui/imageviewer.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <cmath>

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), m_initialScale(1.0), m_pixmapItem(nullptr), m_borderItem(nullptr) // 初始化 m_borderItem
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::HighQualityAntialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
}

void ImageViewer::loadImage(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        setImage(QImage());
        return;
    }
    setImage(image);
}

void ImageViewer::fitToView()
{
    if (!m_pixmapItem) {
        return;
    }
    if (m_pixmapItem->pixmap().isNull()) {
        return;
    }

    resetTransform();
    fitInView(m_pixmapItem, Qt::KeepAspectRatio);

    m_initialScale = transform().m11();
    emit scaleChanged(1.0);
}

void ImageViewer::resetView()
{
    fitToView();
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        // Ctrl + 滚轮缩放，中心为鼠标位置
        if (!m_pixmapItem || m_initialScale <= 0) {
            event->ignore();
            return;
        }

        qreal currentScale = transform().m11() / m_initialScale;
        int numSteps = event->angleDelta().y() / 120;
        qreal factor = std::pow(1.05, numSteps); // 缩放因子

        // 应用相对缩放（使用 AnchorUnderMouse 确保以鼠标位置为中心）
        scale(factor, factor);

        // 计算新相对缩放比例并发出信号
        qreal newScale = currentScale * factor;

        // 可选：限制缩放范围（最小 0.1，最大 10）
        if (newScale < 0.1) {
            newScale = 0.1;
            // 如果超出范围，重置到限制值（这里简化处理，可进一步调整）
            qreal adjustFactor = 0.1 / currentScale;
            resetTransform();
            scale(0.1 * m_initialScale, 0.1 * m_initialScale);
        } else if (newScale > 10) {
            newScale = 10;
            qreal adjustFactor = 10 / currentScale;
            resetTransform();
            scale(10 * m_initialScale, 10 * m_initialScale);
        } else {
            emit scaleChanged(newScale);
        }

        event->accept();
    } else {
        // 无 Ctrl 时，正常滚轮滚动
        QGraphicsView::wheelEvent(event);
    }
}

void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // 窗口大小变化时，重新计算自适应缩放并重置
    QTimer::singleShot(0, this, &ImageViewer::fitToView);
}

void ImageViewer::setScale(qreal scale)
{
    if (!m_pixmapItem || m_initialScale <= 0) return;

    qreal targetM11 = scale * m_initialScale;
    qreal currentM11 = transform().m11();
    if (qFuzzyCompare(currentM11, targetM11)) return;

    resetTransform();
    QGraphicsView::scale(targetM11, targetM11);
    emit scaleChanged(scale);
}

void ImageViewer::updatePixmap(const QPixmap &pixmap)
{
    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(pixmap);
    } else {
    }
}

void ImageViewer::setImage(const QImage &image)
{
    if (image.isNull()) {
        if (m_pixmapItem) m_pixmapItem->setPixmap(QPixmap());
        if (m_borderItem) m_borderItem->setRect(QRectF());
        return;
    }

    QPixmap pixmap = QPixmap::fromImage(image);

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

    QTimer::singleShot(0, this, &ImageViewer::fitToView);
}
