#include "views/imageviewer.h"
#include "viewmodels/imageviewmodel.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <cmath>
#include <QDebug>
#include <QImage>
#include "imageprocessor.h"
#include <QList>
#include "logger.h"
#include "states/drawingstatemachine.h"
#include "items/roi.h"
#include "widgets/extractedimageviewer.h"

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), m_initialScale(1.0), m_pixmapItem(nullptr), m_borderItem(nullptr), m_drawingEnabled(true)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    // setRenderHint(QPainter::HighQualityAntialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);

    // 启用鼠标跟踪，用于像素信息更新
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    // 初始化状态机
    m_drawingStateMachine = new DrawingStateMachine(this, this);
}

void ImageViewer::setViewModel(ImageViewModel *viewModel)
{
    m_viewModel = viewModel;
    connect(m_viewModel, &ImageViewModel::pixmapChanged, this, &ImageViewer::updatePixmap);

    // 当 ViewModel 加载了新图像时，更新此 View 的 imageBounds
    connect(m_viewModel, &ImageViewModel::imageLoaded, this, [this](int, int, int, QRectF imageRect){
        if (imageRect.isNull()) {
            m_imageBounds = QRectF();
            if (m_pixmapItem) m_pixmapItem->setPixmap(QPixmap());
            if (m_borderItem) m_borderItem->setRect(QRectF());
            m_scene->setSceneRect(QRectF());
        } else {
            m_imageBounds = imageRect;
        }
    });
}

void ImageViewer::fitToView()
{
    if (!m_pixmapItem || m_pixmapItem->pixmap().isNull()) {
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
        // Ctrl + 滚轮缩放
        if (!m_pixmapItem || m_initialScale <= 0) {
            event->ignore();
            return;
        }

        qreal currentScale = transform().m11() / m_initialScale;
        int numSteps = event->angleDelta().y() / 120;
        qreal factor = std::pow(1.05, numSteps);

        scale(factor, factor);
        qreal newScale = currentScale * factor;

        // 限制缩放范围
        if (newScale < 0.1) {
            newScale = 0.1;
            resetTransform();
            scale(0.1 * m_initialScale, 0.1 * m_initialScale);
        } else if (newScale > 10) {
            newScale = 10;
            resetTransform();
            scale(10 * m_initialScale, 10 * m_initialScale);
        }
        emit scaleChanged(newScale);
        emit viewZoomed(factor);
        event->accept();
    } else {
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
    if (pixmap.isNull()) {
        if (m_pixmapItem) m_pixmapItem->setPixmap(QPixmap());
        if (m_borderItem) m_borderItem->setRect(QRectF());
        m_scene->setSceneRect(QRectF());
        return;
    }

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

    // 第一次设置 pixmap 时（或加载新图时），自动缩放
    QTimer::singleShot(0, this, &ImageViewer::fitToView);
}

QRectF ImageViewer::imageBounds() const
{
    // 从 m_viewModel 获取
    return m_viewModel ? m_viewModel->imageBounds() : QRectF();
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMousePressEvent(event)) {
    } else {
        QGraphicsView::mousePressEvent(event);
    }
    updatePixelInfo(mapToScene(event->pos()));
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMouseMoveEvent(event)) {
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
    updatePixelInfo(mapToScene(event->pos()));
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMouseReleaseEvent(event)) {
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
    updatePixelInfo(mapToScene(event->pos()));
}

void ImageViewer::translateView(const QPoint& delta)
{
    // 操作滚动条实现平移
    QScrollBar *hBar = horizontalScrollBar();
    QScrollBar *vBar = verticalScrollBar();
    if (hBar && vBar) {
        hBar->setValue(hBar->value() - delta.x());
        vBar->setValue(vBar->value() - delta.y());
        emit viewPanned();
        // qDebug() << "ImageViewer::translateView by" << delta; // qDebug 可能会很多，先注释掉
    } else {
        qWarning() << "ImageViewer::translateView - Scroll bar(s) are null!";
    }
}

void ImageViewer::scaleView(qreal factor)
{
    if (factor <= 0) return;

    QGraphicsView::ViewportAnchor oldAnchor = transformationAnchor();

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    scale(factor, factor);
    setTransformationAnchor(oldAnchor); // 恢复
    qDebug() << "ImageViewer::scaleView by factor" << factor;
    emit viewZoomed(factor);
}

void ImageViewer::panFinished()
{
    // 平移结束后可能需要的操作
    qDebug() << "ImageViewer::panFinished";
    // 例如，如果使用 QGraphicsView::ScrollHandDrag，可能需要设置回 NoDrag
    // setDragMode(QGraphicsView::NoDrag);
}

void ImageViewer::setToolMode(ToolMode mode)
{
    if (m_currentToolMode == mode) return;

    m_currentToolMode = mode;
    qDebug() << "ImageViewer: Tool mode set to" << mode;

    // 如果在绘图时切换回选择模式，应取消当前绘图
    // (我们将在状态机中重命名 DrawingRect 为 Drawing)
    if (mode == ModeSelect &&
        (m_drawingStateMachine->currentState() != DrawingStateMachine::Idle))
    {
        // 强制状态机返回 Idle
        m_drawingStateMachine->setState(DrawingStateMachine::Idle);
    }
}

ImageViewer::ToolMode ImageViewer::currentToolMode() const
{
    return m_currentToolMode;
}

// 内部方法，用于转发像素信息请求
void ImageViewer::updatePixelInfo(const QPointF &scenePos)
{
    if (m_viewModel) {
        m_viewModel->requestPixelInfo(scenePos);
    }
}

/**
 * @brief 槽：当一个ROI请求被提取时调用
 * @param roi 请求提取的ROI实例
 */
void ImageViewer::onExtractRegion(ROI* roi)
{
    if (!roi || !m_viewModel) {
        qWarning() << "onExtractRegion: Received null ROI or ViewModel.";
        return;
    }
    // 从 ViewModel 获取原始图像（假设 ViewModel 有 originalImage() 方法）
    // 注意：如果 ViewModel 没有此方法，需要添加或使用其他方式获取
    QImage originalImage = m_viewModel->imageBounds().isValid() ?
                               ImageProcessor::readRawImg_qImage(":/Resources/img/000006.raw", 2882, 2340) :
                               QImage();

    if (originalImage.isNull()) {
        qWarning() << "onExtractRegion: ViewModel's originalImage is null.";
        return;
    }

    qDebug() << "ImageViewer: Received request to extract from ROI.";

    QImage extractedImage = roi->getArrayRegion(originalImage, true);

    if (extractedImage.isNull()) {
        qWarning() << "onExtractRegion: getArrayRegion returned a null image.";
        return;
    }

    QImage displayImage = extractedImage;

    if (extractedImage.format() == QImage::Format_Grayscale16) {
        int minVal, maxVal;
        ImageProcessor::calculateAutoWindowLevel(extractedImage, minVal, maxVal);
        qDebug() << "Auto window/level for extracted image: Min =" << minVal << "Max =" << maxVal;
        displayImage = ImageProcessor::applyWindowLevel(extractedImage, minVal, maxVal);
    }

    ExtractedImageViewer *viewer = new ExtractedImageViewer(displayImage, this);
    viewer->show();
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMouseDoubleClickEvent(event)) {
        event->accept(); // 状态机处理了它
    } else {
        QGraphicsView::mouseDoubleClickEvent(event); // 基类处理（例如缩放）
    }
}
