// gui/imageviewer.cpp

#include "gui/imageviewer.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer> // 确保 QTimer 被包含

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
    QGraphicsView::wheelEvent(event);
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
