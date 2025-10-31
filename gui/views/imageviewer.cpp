#include "gui/views/imageviewer.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <cmath>
#include <QDebug>
#include <QImage>
#include "service/imageprocessor.h"
#include <QList>
#include "util/logger/logger.h"
#include "gui/states/drawingstatemachine.h"

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

void ImageViewer::loadImage(const QString &filePath)
{
    QImage loadedImage;
    if (filePath.endsWith(".raw", Qt::CaseInsensitive) || filePath.endsWith(".bin", Qt::CaseInsensitive)) {
        // 加载 RAW 图像
        loadedImage = ImageProcessor::readRawImg_qImage(filePath, 2882, 2340);  // 调整尺寸如果需要
        m_imageBounds = loadedImage.rect();
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
        m_imageBounds = QRectF();
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
    m_imageBounds = m_pixmapItem->boundingRect();
    // 初始化窗宽窗位
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    m_windowWidth = maxVal + 1;
    m_windowLevel = maxVal / 2;
    applyWindowLevel();  // 应用初始窗宽窗位

    QTimer::singleShot(0, this, &ImageViewer::fitToView);



}
QRectF ImageViewer::imageBounds() const
{
    return m_imageBounds;
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

// ++ 修改：事件处理函数，转发给状态机 ++
void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMousePressEvent(event)) {
        // 事件已被状态机消耗
    } else {
        log_("aaa=====");
        // 如果状态机未消耗（例如进入 Panning 或 Idle 点击背景），调用基类处理（如果需要的话）
        // 注意：如果 setDragMode(NoDrag)，基类不会处理平移
        QGraphicsView::mousePressEvent(event); // 保留以便基类处理可能的其他交互
    }
    updatePixelInfo(mapToScene(event->pos())); // 总是更新像素信息
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMouseMoveEvent(event)) {
        // 事件已被状态机消耗
    } else {
        // 如果状态机未消耗（例如 Idle 或 ROI 内部处理拖动），调用基类处理
        QGraphicsView::mouseMoveEvent(event);
    }
    updatePixelInfo(mapToScene(event->pos())); // 总是更新像素信息
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_drawingStateMachine->handleMouseReleaseEvent(event)) {
        // 事件已被状态机消耗
    } else {
        // 如果状态机未消耗，调用基类处理
        QGraphicsView::mouseReleaseEvent(event);
    }
    updatePixelInfo(mapToScene(event->pos())); // 总是更新像素信息
}


void ImageViewer::translateView(const QPoint& delta)
{
    // 操作滚动条实现平移
    QScrollBar *hBar = horizontalScrollBar();
    QScrollBar *vBar = verticalScrollBar();
    if (hBar && vBar) {
        hBar->setValue(hBar->value() - delta.x());
        vBar->setValue(vBar->value() - delta.y());
        // qDebug() << "ImageViewer::translateView by" << delta; // qDebug 可能会很多，先注释掉
    } else {
        qWarning() << "ImageViewer::translateView - Scroll bar(s) are null!";
    }
}

void ImageViewer::scaleView(qreal factor)
{
    if (factor <= 0) return;

    // ++ 使用正确的类型 QGraphicsView::ViewportAnchor ++
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
