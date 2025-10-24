#include "gui/views/imageviewer.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <cmath>
#include <QDebug>
#include <QImage>
#include "service/imageprocessor.h"
#include "gui/items/annotationlineitem.h"
#include "gui/items/annotationellipseitem.h"
#include "gui/items/annotationpointitem.h"
#include "gui/items/annotationpointitem.h"
#include <QList>
#include "gui/items/annotationhorizontallineitem.h"
#include "gui/items/annotationverticallineitem.h"
#include "util/logger/logger.h"


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

    // 默认选择模式
    m_currentMode = Mode_Select;
    m_pointItems.clear();  // 初始化

    // 初始化状态机
    m_drawingStateMachine = new DrawingStateMachine(this, this);

}

void ImageViewer::loadImage(const QString &filePath)
{
    QImage loadedImage;
    if (filePath.endsWith(".raw", Qt::CaseInsensitive) || filePath.endsWith(".bin", Qt::CaseInsensitive)) {
        // 加载 RAW 图像（迁移自 MainWindow，使用 ImageProcessor）
        loadedImage = ImageProcessor::readRawImg_qImage(filePath, 2882, 2340);  // 调整尺寸如果需要
    } else {
        // 加载标准图像
        loadedImage.load(filePath);
    }

    if (!loadedImage.isNull()) {
        // 检测位深（迁移自 MainWindow）
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
    // 响应 scale 变化，更新点项
    onScaleChanged(scale);
}

void ImageViewer::onScaleChanged(qreal scale)
{
    // 遍历场景中的所有 Item
    for (QGraphicsItem* item : m_scene->items()) {
        // 根据 Item 的类型调用相应的更新方法
        if (auto pointItem = dynamic_cast<AnnotationPointItem*>(item)) {
            pointItem->updateFontSize(scale); // 点测量
        } else if (auto hLineItem = dynamic_cast<AnnotationHorizontalLineItem*>(item)) {
            hLineItem->updateScale(scale);    // 水平线
        } else if (auto vLineItem = dynamic_cast<AnnotationVerticalLineItem*>(item)) {
            vLineItem->updateScale(scale);    // 垂直线
        }
    }
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

    // 清空旧点项（重新加载图像时）
    for (AnnotationPointItem* item : m_pointItems) {
        if (item) {
            m_scene->removeItem(item);
            delete item;
        }
    }
    m_pointItems.clear();

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

void ImageViewer::setDrawMode(DrawMode mode)
{
    m_currentMode = mode;

    switch (mode) {
    case Mode_Select:
        setDragMode(QGraphicsView::ScrollHandDrag);
        viewport()->setCursor(Qt::OpenHandCursor);
        break;
    case Mode_Line:
    case Mode_Rect:
    case Mode_Ellipse:
    case Mode_Point:
    case Mode_HorizontalLine:
    case Mode_VerticalLine:
    case Mode_WindowLevel:
        setDragMode(QGraphicsView::NoDrag);
        viewport()->setCursor(Qt::CrossCursor);
        break;
    default:
        setDragMode(QGraphicsView::ScrollHandDrag);
        viewport()->setCursor(Qt::ArrowCursor);
        break;
    }
    LogInfo(QString("Draw mode set to: %1").arg(static_cast<int>(mode)));
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
        return; // 如果状态机处理了事件，则直接返回
    }

    // --- 如果状态机未处理，则执行旧逻辑 ---
    QPointF scenePos = mapToScene(event->pos());
    qreal currentAbsoluteScale = (m_initialScale > 0) ? (transform().m11() / m_initialScale) : 1.0;
    int imageWidth = m_originalImage.width();

    switch (m_currentMode) {
    case Mode_Line:
        // m_previewLine = createPreviewLine(scenePos); // (为后续步骤保留)
        break;

    case Mode_Point:
        finishDrawingPoint(scenePos, currentAbsoluteScale, imageWidth);
        break;
    case Mode_HorizontalLine:
    {
        if (m_pixmapItem && !m_pixmapItem->pixmap().isNull()) {
            qreal sceneWidth = m_pixmapItem->boundingRect().width();
            AnnotationHorizontalLineItem *hLine = new AnnotationHorizontalLineItem(scenePos.y(), sceneWidth, currentAbsoluteScale);
            m_scene->addItem(hLine);
        }
    }
    break;
    case Mode_VerticalLine:
    {
        if (m_pixmapItem && !m_pixmapItem->pixmap().isNull()) {
            qreal sceneHeight = m_pixmapItem->boundingRect().height();
            AnnotationVerticalLineItem *vLine = new AnnotationVerticalLineItem(scenePos.x(), sceneHeight, currentAbsoluteScale);
            m_scene->addItem(vLine);
        }
    }
    break;
    case Mode_Select:
    default:
        QGraphicsView::mousePressEvent(event);
        break;
    }
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

    // 如果是选择模式，则执行默认拖动
    if (m_currentMode == Mode_Select) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    switch (m_currentMode) {

    default:
        break;
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

    // --- 其他模式的完成逻辑（除矩形外） ---
    switch (m_currentMode) {

    default:
        break;
    }

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

// 私有方法 - 预览创建
QGraphicsLineItem* ImageViewer::createPreviewLine(const QPointF &start)
{
    QGraphicsLineItem* preview = new QGraphicsLineItem(QLineF(start, start));
    QPen pen(Qt::yellow);
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(2.0);
    preview->setPen(pen);
    preview->setZValue(1000);
    preview->setFlag(QGraphicsItem::ItemIsSelectable, false);
    preview->setFlag(QGraphicsItem::ItemIsMovable, false);
    preview->setAcceptHoverEvents(false);
    m_scene->addItem(preview);
    return preview;
}

QGraphicsRectItem* ImageViewer::createPreviewRect(const QPointF &start)
{
    QGraphicsRectItem* preview = new QGraphicsRectItem(QRectF(start, start));
    QPen pen(Qt::yellow, 1, Qt::DashLine);
    preview->setPen(pen);
    preview->setBrush(Qt::transparent);
    preview->setZValue(1000);
    preview->setFlag(QGraphicsItem::ItemIsSelectable, false);
    preview->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_scene->addItem(preview);
    return preview;
}

// 私有方法 - 完成绘制
void ImageViewer::finishDrawingLine(const QPointF &endPoint)
{
    AnnotationLineItem* line = new AnnotationLineItem(
        m_startPoint.x(), m_startPoint.y(),
        endPoint.x(), endPoint.y()
        );
    m_scene->addItem(line);
    qDebug() << "Line completed from" << m_startPoint << "to" << endPoint;
}

void ImageViewer::finishWindowLevelRect(const QRectF &rect)
{
    if (!rect.isValid()) {
        return;
    }

    // 计算区域 min/max
    int left = qMax(0, qRound(rect.left()));
    int right = qMin(m_originalImage.width(), qRound(rect.right()));
    int top = qMax(0, qRound(rect.top()));
    int bottom = qMin(m_originalImage.height(), qRound(rect.bottom()));

    int minVal = (m_bitDepth == 16) ? 65535 : 255;
    int maxVal = 0;
    bool foundPixel = false;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            if (m_originalImage.valid(x, y)) {
                foundPixel = true;
                int grayValue = getPixelValue(x, y);
                minVal = qMin(minVal, grayValue);
                maxVal = qMax(maxVal, grayValue);
            }
        }
    }

    if (foundPixel && minVal < maxVal) {
        m_windowWidth = maxVal - minVal;
        m_windowLevel = minVal + m_windowWidth / 2;
        applyWindowLevel();
        emit windowLevelChanged(m_windowWidth, m_windowLevel);
        m_autoWindowing = false;
        emit autoWindowingToggled(false);
    }
}

void ImageViewer::switchToSelectMode()
{
    setDrawMode(Mode_Select);
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


// 修改 finishDrawingPoint：传递当前 scale 和图像宽度
void ImageViewer::finishDrawingPoint(const QPointF &pointPos, qreal currentScale, int imageWidth)
{
    int x = qRound(pointPos.x());
    int y = qRound(pointPos.y());
    if (!m_originalImage.valid(x, y)) {
        qDebug() << "Point out of bounds";
        return;
    }
    int value = getPixelValue(x, y);
    AnnotationPointItem *pointItem = new AnnotationPointItem(x, y, value, currentScale, imageWidth);
    m_scene->addItem(pointItem);
    m_pointItems.append(pointItem);  // 跟踪
    qDebug() << "Point measured at" << x << y << "value:" << value << "scale:" << currentScale;
}



void ImageViewer::clearAllAnnotations()
{
    // --- 关键修复：只删除顶层图元 ---

    // 1. 创建一个列表，用来存放所有需要被删除的顶层图元
    QList<QGraphicsItem*> topLevelItemsToRemove;
    for (QGraphicsItem* item : m_scene->items()) {
        // 一个图元是我们自己添加的顶层标注，需要满足以下条件：
        // a) 它没有父级 (item->parentItem() == nullptr)
        // b) 它不是图像本身 (item != m_pixmapItem)
        // c) 它不是图像的边框 (item != m_borderItem)
        if (!item->parentItem() && item != m_pixmapItem && item != m_borderItem) {
            topLevelItemsToRemove.append(item);
        }
    }

    // 2. 遍历这个安全的顶层图元列表，进行删除
    // 这样做可以保证每个图元只被删除一次，并且由 Qt 安全地处理其子图元的销毁
    for (QGraphicsItem* item : topLevelItemsToRemove) {
        m_scene->removeItem(item);
        delete item;
    }

    // 3. 清空我们自己维护的跟踪列表
    m_pointItems.clear();
}

void ImageViewer::setDrawingState(DrawingState *state)
{
    m_drawingStateMachine->setState(state);
}
