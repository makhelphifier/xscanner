// gui/imageviewer.cpp

#include "gui/imageviewer.h"
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QTimer> // 确保 QTimer 被包含

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), m_initialScale(1.0), m_pixmapItem(nullptr), m_borderItem(nullptr) // 初始化 m_borderItem
{
    qDebug() << "[ImageViewer] Constructor started.";
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
    qDebug() << "[ImageViewer] Constructor finished.";
}

void ImageViewer::loadImage(const QString &filePath)
{
    qDebug() << "[ImageViewer] loadImage called with path:" << filePath;
    QImage image(filePath);
    if (image.isNull()) {
        qDebug() << "!!!!!! [ImageViewer] CRITICAL: loadImage failed, QImage is null.";
        setImage(QImage());
        return;
    }
    setImage(image);
}

void ImageViewer::fitToView()
{
    qDebug() << "--- [ImageViewer] fitToView slot started ---";
    if (!m_pixmapItem) {
        qDebug() << "!!!!!! [ImageViewer] WARNING: fitToView called but m_pixmapItem is null. Aborting.";
        return;
    }
    if (m_pixmapItem->pixmap().isNull()) {
        qDebug() << "!!!!!! [ImageViewer] WARNING: fitToView called but pixmap is null. Aborting.";
        return;
    }

    qDebug() << "[ImageViewer] fitToView: Resetting transform.";
    resetTransform();
    qDebug() << "[ImageViewer] fitToView: Calling fitInView.";
    fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    qDebug() << "[ImageViewer] fitToView: fitInView finished.";

    m_initialScale = transform().m11();
    qDebug() << "[ImageViewer] fitToView: New initial scale factor calculated:" << m_initialScale;
    emit scaleChanged(1.0);
    qDebug() << "--- [ImageViewer] fitToView slot finished ---";
}

void ImageViewer::resetView()
{
    qDebug() << "[ImageViewer] resetView called.";
    fitToView();
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    // (这部分暂时不加日志, 除非怀疑是滚轮操作导致崩溃)
    QGraphicsView::wheelEvent(event);
}

void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    // (同上)
    QGraphicsView::keyPressEvent(event);
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    qDebug() << "[ImageViewer] resizeEvent started.";
    QGraphicsView::resizeEvent(event);
    qDebug() << "[ImageViewer] resizeEvent: Scheduling fitToView with QTimer.";
    // 窗口大小变化时，重新计算自适应缩放并重置
    QTimer::singleShot(0, this, &ImageViewer::fitToView);
    qDebug() << "[ImageViewer] resizeEvent finished.";
}

void ImageViewer::setScale(qreal scale)
{
    qDebug() << "[ImageViewer] setScale called with scale:" << scale;
    // (这部分暂时不加日志)
    if (!m_pixmapItem || m_initialScale <= 0) return;

    qreal targetM11 = scale * m_initialScale;
    qreal currentM11 = transform().m11();
    if (qFuzzyCompare(currentM11, targetM11)) return;

    resetTransform();
    QGraphicsView::scale(targetM11, targetM11);
}

void ImageViewer::updatePixmap(const QPixmap &pixmap)
{
    qDebug() << "[ImageViewer] updatePixmap called.";
    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(pixmap);
        qDebug() << "[ImageViewer] updatePixmap: Pixmap updated on existing item.";
    } else {
        qDebug() << "!!!!!! [ImageViewer] WARNING: updatePixmap called but m_pixmapItem is null.";
    }
}

void ImageViewer::setImage(const QImage &image)
{
    qDebug() << "--- [ImageViewer] setImage started ---";
    if (image.isNull()) {
        qDebug() << "[ImageViewer] setImage: Image is null, clearing scene.";
        if (m_pixmapItem) m_pixmapItem->setPixmap(QPixmap());
        if (m_borderItem) m_borderItem->setRect(QRectF());
        qDebug() << "--- [ImageViewer] setImage finished (cleared scene). ---";
        return;
    }

    qDebug() << "[ImageViewer] setImage: Converting QImage to QPixmap...";
    QPixmap pixmap = QPixmap::fromImage(image);
    qDebug() << "[ImageViewer] setImage: Conversion finished. Pixmap size:" << pixmap.size();

    if (!m_pixmapItem) {
        qDebug() << "[ImageViewer] setImage: m_pixmapItem is null, creating a new one.";
        m_pixmapItem = new QGraphicsPixmapItem();
        m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
        m_scene->addItem(m_pixmapItem);
        qDebug() << "[ImageViewer] setImage: New pixmap item created and added to scene.";
    }
    m_pixmapItem->setPixmap(pixmap);
    qDebug() << "[ImageViewer] setImage: setPixmap called on m_pixmapItem.";

    if (!m_borderItem) {
        qDebug() << "[ImageViewer] setImage: m_borderItem is null, creating a new one.";
        m_borderItem = new QGraphicsRectItem(m_pixmapItem);
        m_borderItem->setPen(QPen(Qt::white, 2));
        qDebug() << "[ImageViewer] setImage: New border item created.";
    }
    m_borderItem->setRect(m_pixmapItem->boundingRect());
    qDebug() << "[ImageViewer] setImage: Border rectangle set to:" << m_pixmapItem->boundingRect();

    m_scene->setSceneRect(m_pixmapItem->boundingRect());
    qDebug() << "[ImageViewer] setImage: Scene rectangle set to:" << m_pixmapItem->boundingRect();

    qDebug() << "[ImageViewer] setImage: Scheduling fitToView with QTimer.";
    QTimer::singleShot(0, this, &ImageViewer::fitToView);

    qDebug() << "--- [ImageViewer] setImage finished. ---";
}
