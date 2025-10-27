#include "gui/views/imageviewer.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <cmath>
#include <QDebug>
#include <QImage>
#include "service/imageprocessor.h"
#include <QList>
// #include "util/logger/logger.h"


ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), m_initialScale(1.0), m_pixmapItem(nullptr), m_borderItem(nullptr)
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

    // 启用鼠标跟踪，用于像素信息更新
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    // 初始化状态机
    m_drawingStateMachine = new DrawingStateMachine(this, this);
}

void ImageViewer::loadImage(const QString &filePath)
{
    QImage loadedImage;
    if (filePath.endsWith(".raw", Qt::CaseInsensitive) || filePath.endsWith(".bin", Qt::CaseInsensitive)) {
        // 加载 RAW 图像
        loadedImage = ImageProcessor::readRawImg_qImage(filePath, 2882, 2340);  // 调整尺寸如果需要
    } else {
        // 加载标准图像
        loadedImage.load(filePath);
    }
    if (!loadedImage.isNull()) {
        // 检测位深
        m_bitDepth = (loadedImage.format() == QImage::Format_Grayscale16) ? 16 : 8;
        // 如果是 RGB，转换为灰度（假设 8 位）
        if (!loadedImage.isGrayscale()) {
            loadedImage = loadedImage.convertToFormat(QImage::Format_Grayscale8);
            m_bitDepth = 8;
        }
        m_originalImage = loadedImage;
    }

    setImage(loadedImage);
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
    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(pixmap);
        if (m_borderItem) {
            m_borderItem->setRect(m_pixmapItem->boundingRect());
        }
        m_scene->setSceneRect(m_pixmapItem->boundingRect());
    }
}

void ImageViewer::setImage(const QImage &image)
{
    if (image.isNull()) {
        if (m_pixmapItem) m_pixmapItem->setPixmap(QPixmap());
        if (m_borderItem) m_borderItem->setRect(QRectF());
        m_originalImage = QImage();
        m_scene->setSceneRect(QRectF());
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

    // 初始化窗宽窗位（迁移自 MainWindow）
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    m_windowWidth = maxVal + 1;
    m_windowLevel = maxVal / 2;
    applyWindowLevel();  // 应用初始窗宽窗位

    QTimer::singleShot(0, this, &ImageViewer::fitToView);



}

// 公共接口 - 窗宽窗位
void ImageViewer::setWindowLevel(int width, int level)
{
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    m_windowWidth = qBound(1, width, maxVal + 1);
    m_windowLevel = qBound(0, level, maxVal);
    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
}

void ImageViewer::setAutoWindowing(bool enabled)
{
    m_autoWindowing = enabled;
    emit autoWindowingToggled(enabled);

    if (m_originalImage.isNull()) return;

    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    if (enabled) {
        int min, max;
        calculateAutoWindowLevel(min, max);  // 使用 ImageProcessor 计算
        m_windowWidth = max - min;
        m_windowLevel = min + m_windowWidth / 2;
    } else {
        // 恢复全范围
        m_windowWidth = maxVal + 1;
        m_windowLevel = maxVal / 2;
    }
    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
}

void ImageViewer::applyWindowLevel()
{
    if (m_originalImage.isNull()) return;

    int min = m_windowLevel - m_windowWidth / 2;
    int max = m_windowLevel + m_windowWidth / 2;
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    min = qBound(0, min, maxVal);
    max = qBound(0, max, maxVal);
    if (min > max) std::swap(min, max);

    // 使用 ImageProcessor 应用窗宽窗位
    QImage adjustedImage = ImageProcessor::applyWindowLevel(m_originalImage, min, max);
    updatePixmap(QPixmap::fromImage(adjustedImage));
}



// 槽 - 从 UI 接收窗宽/窗位变化
void ImageViewer::onWindowChanged(int value)
{
    setWindowLevel(value, m_windowLevel);  // 更新窗宽，保持窗位
    m_autoWindowing = false;  // 手动调整取消自动
    emit autoWindowingToggled(false);
}

void ImageViewer::onLevelChanged(int value)
{
    setWindowLevel(m_windowWidth, value);  // 更新窗位，保持窗宽
    m_autoWindowing = false;  // 手动调整取消自动
    emit autoWindowingToggled(false);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if (m_originalImage.isNull()) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    // 将事件传递给状态机
    m_drawingStateMachine->handleMousePress(event);
    if (event->isAccepted()) {
        return;
    }

    QPointF scenePos = mapToScene(event->pos());

    updatePixelInfo(scenePos);
}


void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    updatePixelInfo(scenePos);  // 始终更新像素信息

    // 将事件传递给状态机
    m_drawingStateMachine->handleMouseMove(event);
    if (event->isAccepted()) {
        return;
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || m_originalImage.isNull()) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    // 将事件传递给状态机
    m_drawingStateMachine->handleMouseRelease(event);
    if (event->isAccepted()) {
        return;
    }

    QPointF scenePos = mapToScene(event->pos());

    updatePixelInfo(scenePos);
}


// 私有方法 - 像素信息更新
void ImageViewer::updatePixelInfo(const QPointF &scenePos)
{
    int x = qRound(scenePos.x());
    int y = qRound(scenePos.y());
    int value = -1;  // N/A 默认

    if (m_pixmapItem && m_pixmapItem->boundingRect().contains(scenePos) && m_originalImage.valid(x, y)) {
        value = getPixelValue(x, y);
    }

    emit pixelInfoChanged(x, y, value);
}

int ImageViewer::getPixelValue(int x, int y) const
{
    if (m_bitDepth == 16) {
        const quint16* scanLine = reinterpret_cast<const quint16*>(m_originalImage.constScanLine(y));
        return scanLine[x];
    } else {
        return qGray(m_originalImage.pixel(x, y));
    }
}


// 私有方法 - 自动窗宽窗位计算
void ImageViewer::calculateAutoWindowLevel(int &min, int &max)
{
    ImageProcessor::calculateAutoWindowLevel(m_originalImage, min, max);
}

int ImageViewer::currentWindowWidth() const
{
    return m_windowWidth;
}

int ImageViewer::currentWindowLevel() const
{
    return m_windowLevel;
}

void ImageViewer::setDrawingState(DrawingState *state)
{
    m_drawingStateMachine->setState(state);
}
