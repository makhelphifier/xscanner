#include "imagegraphicsview.h"
#include <QActionGroup>
#include <QWidgetAction>
#include <QFileInfo>
#include <QPolygonF>
#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <algorithm>
#include <vector>
#include <cmath>

// 常量定义
const double ImageGraphicsView::MIN_SCALE = 0.1;
const double ImageGraphicsView::MAX_SCALE = 10.0;

// AnnotationItem 实现
AnnotationItem::AnnotationItem(AnnotationType type, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_type(type)
    , m_pen(QPen(Qt::red, 2))
    , m_brush(QBrush(Qt::transparent))
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
}

// LineAnnotation 实现
LineAnnotation::LineAnnotation(QGraphicsItem* parent)
    : AnnotationItem(Line, parent)
{
}

QRectF LineAnnotation::boundingRect() const
{
    qreal extra = (m_pen.width() + 1) / 2.0;
    return QRectF(m_startPoint, m_endPoint).normalized().adjusted(-extra, -extra, extra, extra);
}

void LineAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    painter->setPen(m_pen);
    painter->drawLine(m_startPoint, m_endPoint);
    
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

// RectangleAnnotation 实现
RectangleAnnotation::RectangleAnnotation(QGraphicsItem* parent)
    : AnnotationItem(Rectangle, parent)
{
}

QRectF RectangleAnnotation::boundingRect() const
{
    qreal extra = (m_pen.width() + 1) / 2.0;
    return QRectF(m_startPoint, m_endPoint).normalized().adjusted(-extra, -extra, extra, extra);
}

void RectangleAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawRect(QRectF(m_startPoint, m_endPoint).normalized());
    
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

// EllipseAnnotation 实现
EllipseAnnotation::EllipseAnnotation(QGraphicsItem* parent)
    : AnnotationItem(Ellipse, parent)
{
}

QRectF EllipseAnnotation::boundingRect() const
{
    qreal extra = (m_pen.width() + 1) / 2.0;
    return QRectF(m_startPoint, m_endPoint).normalized().adjusted(-extra, -extra, extra, extra);
}

void EllipseAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawEllipse(QRectF(m_startPoint, m_endPoint).normalized());

    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

// ArrowAnnotation 实现
ArrowAnnotation::ArrowAnnotation(QGraphicsItem* parent)
    : AnnotationItem(Arrow, parent)
{
}

QRectF ArrowAnnotation::boundingRect() const
{
    qreal extra = (m_pen.width() + 20) / 2.0; // 额外空间用于箭头
    return QRectF(m_startPoint, m_endPoint).normalized().adjusted(-extra, -extra, extra, extra);
}

void ArrowAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(m_pen);
    painter->setBrush(QBrush(m_pen.color()));

    // 绘制线条
    painter->drawLine(m_startPoint, m_endPoint);

    // 绘制箭头
    QPolygonF arrowHead = createArrowHead(m_startPoint, m_endPoint);
    painter->drawPolygon(arrowHead);

    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

QPolygonF ArrowAnnotation::createArrowHead(const QPointF& start, const QPointF& end) const
{
    QPolygonF arrowHead;

    if (start == end) {
        return arrowHead;
    }

    // 计算箭头的方向和大小
    QPointF direction = end - start;
    double length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());

    if (length == 0) {
        return arrowHead;
    }

    direction /= length; // 单位向量

    double arrowLength = 15.0;
    double arrowAngle = 0.5; // 弧度

    // 计算箭头的两个边
    QPointF arrowP1 = end - arrowLength * (direction * cos(arrowAngle) +
                                          QPointF(-direction.y(), direction.x()) * sin(arrowAngle));
    QPointF arrowP2 = end - arrowLength * (direction * cos(arrowAngle) -
                                          QPointF(-direction.y(), direction.x()) * sin(arrowAngle));

    arrowHead << end << arrowP1 << arrowP2;
    return arrowHead;
}

// TextAnnotation 实现
TextAnnotation::TextAnnotation(const QString& text, QGraphicsItem* parent)
    : AnnotationItem(Text, parent)
    , m_text(text)
    , m_font(QFont("Arial", 12))
{
}

QRectF TextAnnotation::boundingRect() const
{
    QFontMetrics metrics(m_font);
    QRectF rect = metrics.boundingRect(m_text);
    rect.moveTopLeft(m_startPoint);
    qreal extra = m_pen.width() / 2.0;
    return rect.adjusted(-extra, -extra, extra, extra);
}

void TextAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(m_pen);
    painter->setFont(m_font);
    painter->drawText(m_startPoint, m_text);

    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

// MeasurementAnnotation 实现
MeasurementAnnotation::MeasurementAnnotation(QGraphicsItem* parent)
    : AnnotationItem(Measurement, parent)
    , m_pixelScale(1.0)
    , m_unit("px")
{
}

QRectF MeasurementAnnotation::boundingRect() const
{
    qreal extra = (m_pen.width() + 30) / 2.0; // 额外空间用于文本和标记
    return QRectF(m_startPoint, m_endPoint).normalized().adjusted(-extra, -extra, extra, extra);
}

void MeasurementAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(m_pen);

    // 绘制测量线
    painter->drawLine(m_startPoint, m_endPoint);

    // 绘制端点标记
    qreal markSize = 5.0;
    painter->drawLine(m_startPoint + QPointF(-markSize, -markSize),
                     m_startPoint + QPointF(markSize, markSize));
    painter->drawLine(m_startPoint + QPointF(-markSize, markSize),
                     m_startPoint + QPointF(markSize, -markSize));

    painter->drawLine(m_endPoint + QPointF(-markSize, -markSize),
                     m_endPoint + QPointF(markSize, markSize));
    painter->drawLine(m_endPoint + QPointF(-markSize, markSize),
                     m_endPoint + QPointF(markSize, -markSize));

    // 绘制距离文本
    QString distanceText = getDistanceText();
    QFontMetrics metrics(painter->font());
    QRectF textRect = metrics.boundingRect(distanceText);

    QPointF midPoint = (m_startPoint + m_endPoint) / 2.0;
    textRect.moveCenter(midPoint);

    // 绘制文本背景
    painter->setBrush(QBrush(Qt::white));
    painter->drawRect(textRect.adjusted(-2, -2, 2, 2));

    // 绘制文本
    painter->setBrush(Qt::NoBrush);
    painter->drawText(textRect, Qt::AlignCenter, distanceText);

    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

double MeasurementAnnotation::getDistance() const
{
    QPointF diff = m_endPoint - m_startPoint;
    double pixelDistance = sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    return pixelDistance * m_pixelScale;
}

QString MeasurementAnnotation::getDistanceText() const
{
    double distance = getDistance();
    return QString("%1 %2").arg(distance, 0, 'f', 2).arg(m_unit);
}

// ROIOverlayWidget 实现
ROIOverlayWidget::ROIOverlayWidget(QWidget* parent)
    : QWidget(parent)
    , m_isDrawing(false)
    , m_imageInfoPos(10, 10)
    , m_mouseInfoPos(10, 100)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setStyleSheet("background: transparent;");

    // 设置字体
    m_infoFont = QFont("Consolas", 9);
    if (!m_infoFont.exactMatch()) {
        m_infoFont = QFont("Monaco", 9);
        if (!m_infoFont.exactMatch()) {
            m_infoFont = QFont("monospace", 9);
        }
    }
}

void ROIOverlayWidget::setROIRect(const QRect& rect)
{
    m_roiRect = rect;
    update();
}

void ROIOverlayWidget::clearROI()
{
    m_roiRect = QRect();
    m_isDrawing = false;
    update();
}

void ROIOverlayWidget::setImageInfo(const QString& info)
{
    m_imageInfo = info;
    update();
}

void ROIOverlayWidget::setMouseInfo(const QString& info)
{
    m_mouseInfo = info;
    update();
}

void ROIOverlayWidget::setImageInfoPosition(const QPoint& pos)
{
    m_imageInfoPos = pos;
    update();
}

void ROIOverlayWidget::setMouseInfoPosition(const QPoint& pos)
{
    m_mouseInfoPos = pos;
    update();
}

void ROIOverlayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 绘制ROI矩形区域（如果正在绘制）
    if (m_isDrawing && !m_roiRect.isEmpty()) {
        painter.setPen(QPen(Qt::yellow, 2, Qt::SolidLine));
        painter.setBrush(QBrush(QColor(255, 255, 0, 80))); // 黄色半透明背景
        painter.drawRect(m_roiRect);
    }

    // 绘制图像信息
    if (!m_imageInfo.isEmpty()) {
        drawInfoText(painter, m_imageInfo, m_imageInfoPos);
    }

    // 绘制鼠标信息
    if (!m_mouseInfo.isEmpty()) {
        drawInfoText(painter, m_mouseInfo, m_mouseInfoPos);
    }
}

void ROIOverlayWidget::drawInfoText(QPainter& painter, const QString& text, const QPoint& pos)
{
    if (text.isEmpty()) return;

    painter.setFont(m_infoFont);

    // 计算文本尺寸
    QFontMetrics fm(m_infoFont);
    QRect textRect = fm.boundingRect(text);

    // 添加内边距
    int padding = 8;
    QRect backgroundRect = textRect.adjusted(-padding, -padding, padding, padding);
    backgroundRect.moveTopLeft(pos);

    // 绘制半透明背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(0, 0, 0, 180)));
    painter.drawRoundedRect(backgroundRect, 4, 4);

    // 绘制文本
    painter.setPen(QPen(Qt::white));
    painter.setBrush(Qt::NoBrush);
    QRect drawRect = backgroundRect.adjusted(padding, padding, -padding, -padding);
    painter.drawText(drawRect, Qt::AlignLeft | Qt::AlignTop, text);
}




QImage AutoAdjust(const QImage& image, int autoThreshold, int& ww, int& wl)
{
    if (image.format() != QImage::Format_Grayscale16) {
        qWarning() << "Image format is not Grayscale16, cannot apply auto adjust";
        return image;
    }

    QImage resultImage = image.copy();
    int width = image.width();
    int height = image.height();
    int pixelCount = width * height;
    int limit = pixelCount / 10;

    // Find min and max values in the image
    const quint16* imageData = reinterpret_cast<const quint16*>(image.constBits());
    quint16 minVal = 65535;
    quint16 maxVal = 0;

    for (int i = 0; i < pixelCount; i++) {
        quint16 pixel = imageData[i];
        if (pixel < minVal) minVal = pixel;
        if (pixel > maxVal) maxVal = pixel;
    }

    // Create histogram with 256 bins
    QVector<int> hist(256, 0);
    double binSize = (maxVal - minVal + 0.0) / 256.0;
    double scale = 256.0 / (maxVal - minVal);

    // Generate histogram
    for (int i = 0; i < pixelCount; i++) {
        quint16 pixel = imageData[i];
        if (pixel >= minVal && pixel <= maxVal) {
            int index = static_cast<int>(scale * (pixel - minVal));
            if (index > 255) index = 255;
            if (index < 0) index = 0;
            hist[index]++;
        }
    }

    int m_autoThreshold = autoThreshold;
    int threshold = pixelCount / m_autoThreshold;

    // Find hmin
    int hmin = 0;
    bool found = false;
    for (int i = 0; i < 256 && !found; i++) {
        int count = hist[i];
        if (count > limit) count = 0;
        found = count > threshold;
        if (!found) hmin = i + 1;
    }

    // Find hmax
    int hmax = 255;
    found = false;
    for (int i = 255; i >= 0 && !found; i--) {
        int count = hist[i];
        if (count > limit) count = 0;
        found = count > threshold;
        if (!found) hmax = i - 1;
    }

    if (hmax >= hmin) {
        double min = minVal + hmin * binSize;
        double max = minVal + hmax * binSize;

        if (min == max) {
            min = minVal;
            max = maxVal;
        }

        ww = static_cast<int>(max - min);
        wl = static_cast<int>((max + min) / 2);

        double scale1 = 65535.0 / (max - min);

        // Apply the transformation to result image
        quint16* resultData = reinterpret_cast<quint16*>(resultImage.bits());
        for (int i = 0; i < pixelCount; i++) {
            double value = imageData[i];
            value = (value - min) * scale1;

            if (value < 0.0) {
                value = 0.0;
            } else if (value > 65535.0) {
                value = 65535.0;
            }

            resultData[i] = static_cast<quint16>(value + 0.5);
        }
    }

    return resultImage;
}



// ImageGraphicsView 实现
ImageGraphicsView::ImageGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , m_graphicsScene(nullptr)
    , m_pixmapItem(nullptr)
    , m_interactionMode(Pan)
    , m_annotationTool(LineTool)
    , m_annotationPen(QPen(Qt::red, 2))
    , m_annotationBrush(QBrush(Qt::transparent))
    , m_isDrawing(false)
    , m_currentAnnotation(nullptr)
    , m_isPanning(false)
    , m_scaleFactor(1.0)
    , m_spacePressed(false)
    , m_originalCursor(Qt::ArrowCursor)
    , m_isRightDragging(false)
    , m_roiOverlay(nullptr)
    , m_windowLevelCalculationWatcher(nullptr)
    , m_windowLevelApplicationWatcher(nullptr)
    , m_autoApplyAfterCalculation(false)
    , m_hasPendingImage(false)
    , m_isLoadingImage(false)
{
    // 创建图形场景
    m_graphicsScene = new QGraphicsScene(this);
    setScene(m_graphicsScene);
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // 启用鼠标跟踪
    setMouseTracking(true);

    // 启用键盘焦点，以便接收键盘事件
    setFocusPolicy(Qt::StrongFocus);

    // 设置上下文菜单策略
    setContextMenuPolicy(Qt::CustomContextMenu);
    // connect(this, &QWidget::customContextMenuRequested, this, &ImageGraphicsView::showContextMenu);

    // 先创建ROI蒙层控件
    m_roiOverlay = new ROIOverlayWidget(this);
    m_roiOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // m_roiOverlay->hide();
    connect(m_roiOverlay, &ROIOverlayWidget::roiFinished, this, &ImageGraphicsView::rectangleSelected);

    // 设置信息显示的初始位置和内容
    m_roiOverlay->setImageInfoPosition(QPoint(10, 10));
    m_roiOverlay->setImageInfo("No Image");
    m_roiOverlay->setMouseInfo("Mouse: ---, --- | RGB: ---, ---, ---");

    // 初始化信息显示
    updateInfoLabels();

    // 初始化窗位窗宽计算相关的 FutureWatcher
    m_windowLevelCalculationWatcher = new QFutureWatcher<WindowLevelResult>(this);
    connect(m_windowLevelCalculationWatcher, &QFutureWatcher<WindowLevelResult>::finished,
            this, &ImageGraphicsView::onWindowLevelCalculationFinished);

    m_windowLevelApplicationWatcher = new QFutureWatcher<QImage>(this);
    connect(m_windowLevelApplicationWatcher, &QFutureWatcher<QImage>::finished,
            this, &ImageGraphicsView::onWindowLevelApplicationFinished);

    // 图像加载定时器已移除，现在使用直接加载方式

    // 初始化pixmap item，确保始终有一个固定的item用于显示图像
    initializePixmapItem();

    // 连接跨线程图像加载信号槽
    connect(this, &ImageGraphicsView::imageLoadRequested,
            this, &ImageGraphicsView::onImageLoadRequested, Qt::QueuedConnection);
    connect(this, &ImageGraphicsView::imageLoadHighFrequencyRequested,
            this, &ImageGraphicsView::onImageLoadHighFrequencyRequested, Qt::QueuedConnection);
}

ImageGraphicsView::~ImageGraphicsView()
{
    // 等待任何正在进行的图像加载完成
    QMutexLocker locker(&m_imageLoadMutex);

    if (m_graphicsScene) {
        delete m_graphicsScene;
    }
}

void ImageGraphicsView::setInteractionMode(InteractionMode mode)
{
    m_interactionMode = mode;

    // 如果空格键没有按下，才更新光标
    if (!m_spacePressed) {
        updateCursor();
    }
}

void ImageGraphicsView::updateCursor()
{
    if (m_spacePressed) {
        setCursor(Qt::OpenHandCursor);
        m_originalCursor = Qt::OpenHandCursor;
    } else {
        switch (m_interactionMode) {
        case Pan:
            setCursor(Qt::ArrowCursor);
            m_originalCursor = Qt::ArrowCursor;
            break;
        case Annotate:
        case Measure:
            setCursor(Qt::CrossCursor);
            m_originalCursor = Qt::CrossCursor;
            break;
        }
    }
}

void ImageGraphicsView::setAnnotationTool(AnnotationTool tool)
{
    m_annotationTool = tool;
}

void ImageGraphicsView::clearAnnotations()
{
    for (AnnotationItem* item : m_annotations) {
        m_graphicsScene->removeItem(item);
        delete item;
    }
    m_annotations.clear();
}

void ImageGraphicsView::loadImage(const QPixmap& pixmap)
{
    // 使用互斥锁保护，防止并发访问
    QMutexLocker locker(&m_imageLoadMutex);

    // 如果正在加载图像，直接更新待处理的图像
    if (m_isLoadingImage) {
        m_pendingPixmap = pixmap;
        m_hasPendingImage = true;
        return;
    }

    // 标记正在加载
    m_isLoadingImage = true;
    m_hasPendingImage = false;

    // 释放锁，避免在UI操作时持有锁
    locker.unlock();


    // 直接执行图像加载，不使用定时器
    loadImageInternal(pixmap);

    // 重新获取锁并重置状态
    locker.relock();
    m_isLoadingImage = false;

    // 检查是否有待处理的图像
    if (m_hasPendingImage) {
        QPixmap pendingPixmap = m_pendingPixmap;
        m_hasPendingImage = false;
        m_isLoadingImage = true;

        locker.unlock();
        loadImageInternal(pendingPixmap);
        locker.relock();
        m_isLoadingImage = false;
    }
}

void ImageGraphicsView::loadImageImmediate(const QPixmap& pixmap)
{
    QMutexLocker locker(&m_imageLoadMutex);
    m_hasPendingImage = false;
    m_isLoadingImage = true;
    locker.unlock();

    // 立即执行图像加载
    loadImageInternal(pixmap);

    locker.relock();
    m_isLoadingImage = false;
}

void ImageGraphicsView::loadImageHighFrequency(const QPixmap& pixmap)
{
    // 高频率图像加载的优化版本，专门用于每秒100+张图像的场景

    // 使用tryLock避免阻塞，如果无法获取锁则跳过这次更新
    if (!m_imageLoadMutex.tryLock()) {
        return; // 如果正在加载，跳过这次更新
    }

    // 如果正在加载，只更新待处理的图像
    if (m_isLoadingImage) {
        m_pendingPixmap = pixmap;
        m_hasPendingImage = true;
        m_imageLoadMutex.unlock();
        return;
    }

    m_isLoadingImage = true;
    m_hasPendingImage = false;
    m_imageLoadMutex.unlock();

    // 确保在主线程中执行
    if (QThread::currentThread() != this->thread()) {
        emit imageLoadHighFrequencyRequested(pixmap);
        QMutexLocker locker(&m_imageLoadMutex);
        m_isLoadingImage = false;
        return;
    }

    // 确保pixmap item存在
    if (!m_pixmapItem) {
        initializePixmapItem();
    }

    // 高性能图像更新：只更新pixmap，不重建场景
    if (m_pixmapItem) {
        // 直接更新现有item的pixmap，这是最快的方式
        if (!pixmap.isNull()) {
            m_pixmapItem->setPixmap(pixmap);

            // 只在图像尺寸改变时才更新场景矩形
            QRectF newRect = pixmap.rect();
            if (m_graphicsScene->sceneRect() != newRect) {
                m_graphicsScene->setSceneRect(newRect);
            }
        } else {
            // 即使pixmap为空，也只是更新为空pixmap，不移除item
            m_pixmapItem->setPixmap(QPixmap());
        }

        // 更新内部状态
        m_originPixmap = pixmap;
        m_currentPixmap = pixmap;

        // 跳过信息标签更新以提高性能（可选）
        // updateInfoLabels();
    }

    QMutexLocker locker(&m_imageLoadMutex);
    m_isLoadingImage = false;

    // 检查是否有待处理的图像
    if (m_hasPendingImage) {
        QPixmap pendingPixmap = m_pendingPixmap;
        m_hasPendingImage = false;
        m_isLoadingImage = true;

        locker.unlock();

        // 递归处理待处理的图像
        if (m_pixmapItem && !pendingPixmap.isNull()) {
            m_pixmapItem->setPixmap(pendingPixmap);
            if (m_originPixmap.size() != pendingPixmap.size()) {
                m_graphicsScene->setSceneRect(pendingPixmap.rect());
            }
            m_originPixmap = pendingPixmap;
            m_currentPixmap = pendingPixmap;
        }

        locker.relock();
        m_isLoadingImage = false;
    }
}

// 定时器超时处理方法已移除，现在直接处理图像加载

void ImageGraphicsView::loadImageInternal(const QPixmap& pixmap)
{
    // 确保在主线程中执行
    if (QThread::currentThread() != this->thread()) {
        emit imageLoadRequested(pixmap);
        return;
    }

    // 暂时禁用视图更新，减少重绘
    setUpdatesEnabled(false);

    try {
        // 确保始终有一个pixmap item存在，避免频繁添加/移除
        if (!m_pixmapItem) {
            // 只在第一次或item被意外删除时创建
            initializePixmapItem();
        }

        if (m_pixmapItem) {
            if (!pixmap.isNull()) {
                // 直接更新现有item的pixmap，这是最高效的方式
                m_pixmapItem->setPixmap(pixmap);

                // 只在图像尺寸改变时才更新场景矩形
                QRectF newRect = pixmap.rect();
                if (m_graphicsScene->sceneRect() != newRect) {
                    m_graphicsScene->setSceneRect(newRect);
                }
            } else {
                // 即使pixmap为空，也保持item存在，只是设置为空pixmap
                m_pixmapItem->setPixmap(QPixmap());
                m_graphicsScene->setSceneRect(QRectF(0, 0, 100, 100)); // 默认小尺寸
            }
        }

        // 更新内部状态
        m_originPixmap = pixmap;
        m_currentPixmap = pixmap;

        // 直接更新信息标签，不使用定时器
        updateInfoLabels();

    } catch (const std::exception& e) {
        qWarning() << "Exception occurred during image loading:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception occurred during image loading";
    }

    // 重新启用视图更新
    setUpdatesEnabled(true);

    // 强制一次更新以确保显示
    update();
}

void ImageGraphicsView::initializePixmapItem()
{
    // 确保只创建一次pixmap item，避免重复创建
    if (m_pixmapItem) {
        return; // 已经存在，无需重复创建
    }

    // 创建一个空的pixmap item作为占位符
    QPixmap emptyPixmap(1, 1);
    emptyPixmap.fill(Qt::transparent);

    // 添加到场景中
    m_pixmapItem = m_graphicsScene->addPixmap(emptyPixmap);

    if (m_pixmapItem) {
        // 设置item属性
        m_pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        m_pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

        // 设置初始场景矩形
        m_graphicsScene->setSceneRect(QRectF(0, 0, 100, 100));

        qDebug() << "Pixmap item initialized successfully";
    } else {
        qWarning() << "Failed to create pixmap item";
    }
}

void ImageGraphicsView::clearImage()
{
    // 清除图像但保持pixmap item存在，避免频繁添加/移除组件
    if (m_pixmapItem) {
        // 设置为空pixmap而不是删除item
        m_pixmapItem->setPixmap(QPixmap());
    } else {
        // 如果item不存在，创建一个
        initializePixmapItem();
    }

    // 设置默认场景矩形
    m_graphicsScene->setSceneRect(QRectF(0, 0, 100, 100));

    // 清除内部状态
    m_originPixmap = QPixmap();
    m_currentPixmap = QPixmap();

    // 更新信息显示
    updateInfoLabels();

    // 更新视图
    update();
}

void ImageGraphicsView::onImageLoadRequested(const QPixmap& pixmap)
{
    // 这个槽函数总是在主线程中执行
    loadImageInternal(pixmap);
}

void ImageGraphicsView::onImageLoadHighFrequencyRequested(const QPixmap& pixmap)
{
    // 这个槽函数总是在主线程中执行
    loadImageHighFrequency(pixmap);
}

void ImageGraphicsView::setCurrentImage(const QPixmap& pixmap)
{
    m_currentPixmap = pixmap;

    // 直接更新信息标签，不使用定时器
    updateInfoLabels();
}

void ImageGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // 缩放
        const double scaleFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            scaleView(scaleFactor);
        } else {
            scaleView(1.0 / scaleFactor);
        }
        event->accept();
    } else {
        // 默认滚动行为
        QGraphicsView::wheelEvent(event);
    }
}

void ImageGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        if (m_spacePressed) {
            // 空格键按下时，开始平移
            m_lastPanPoint = event->pos();
            m_isPanning = true;
            setCursor(Qt::ClosedHandCursor);
        } else {
            switch (m_interactionMode) {
            case Pan:
                // 平移模式下，不允许拖拽移动图像（除非按住空格键）
                QGraphicsView::mousePressEvent(event);
                break;
            case Annotate:
            case Measure:
                // 标注和测量模式下，只记录坐标，不改变视图位置
                startAnnotation(scenePos);
                break;
            }
        }
    } else if (event->button() == Qt::RightButton) {
        // 右键按下处理
        QPointF scenePos = mapToScene(event->pos());

        // 检查是否在图像区域内
        if (isPointInImage(scenePos)) {
            m_rightClickStart = event->pos();
            m_rightDragStart = event->pos();
            m_rightDragCurrent = event->pos();
            m_isRightDragging = false; // 初始状态不是拖拽
        }
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void ImageGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    QPointF scenePos = mapToScene(event->pos());

    // 更新鼠标位置信息
    updateMousePosition(event->pos());

    if (m_isPanning && (event->buttons() & Qt::LeftButton) && m_spacePressed) {
        // 只有在空格键按下时才允许平移视图
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();

        QScrollBar* hBar = horizontalScrollBar();
        QScrollBar* vBar = verticalScrollBar();
        hBar->setValue(hBar->value() - delta.x());
        vBar->setValue(vBar->value() - delta.y());
    } else if (m_isDrawing && m_currentAnnotation && !m_spacePressed) {
        // 只有在空格键没有按下时才更新标注
        updateAnnotation(scenePos);
    } else if (event->buttons() & Qt::RightButton) {
        // 右键拖拽处理
        QPointF scenePos = mapToScene(event->pos());
        if (isPointInImage(scenePos)) {
            // 检查是否开始拖拽（移动距离超过阈值）
            QPoint currentPos = event->pos();
            QPoint delta = currentPos - m_rightClickStart;
            if (!m_isRightDragging && (abs(delta.x()) > 3 || abs(delta.y()) > 3)) {
                // 开始拖拽，启动ROI绘制
                m_isRightDragging = true;
                startROIDrawing(m_rightDragStart);
            }

            if (m_isRightDragging) {
                // 更新ROI绘制
                m_rightDragCurrent = currentPos;
                updateROIDrawing(m_rightDragCurrent);
            }
        }
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void ImageGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isPanning) {
            m_isPanning = false;
            // 恢复到适当的光标
            updateCursor();
        } else if (m_isDrawing && !m_spacePressed) {
            // 只有在空格键没有按下时才完成标注
            QPointF scenePos = mapToScene(event->pos());
            finishAnnotation(scenePos);
        }
    } else if (event->button() == Qt::RightButton) {
        // 右键抬起处理
        if (m_isRightDragging) {
            // 完成ROI绘制
            finishROIDrawing();
        } else {
            // 原地点击，检查是否在图像上才显示上下文菜单
            QPointF scenePos = mapToScene(event->pos());
            if (isPointInImage(scenePos)) {
                // showContextMenu(event->pos());
            }
        }

        // 清理右键拖拽状态
        m_isRightDragging = false;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void ImageGraphicsView::contextMenuEvent(QContextMenuEvent* event)
{
    // 不在这里处理上下文菜单，在 mouseReleaseEvent 中处理
    event->ignore();
}

void ImageGraphicsView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        if (!m_spacePressed) {
            m_spacePressed = true;
            updateCursor();
        }
        event->accept();
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

void ImageGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        if (m_spacePressed) {
            m_spacePressed = false;
            m_isPanning = false; // 停止任何正在进行的平移
            updateCursor();
        }
        event->accept();
    } else {
        QGraphicsView::keyReleaseEvent(event);
    }
}

void ImageGraphicsView::showContextMenu(const QPoint& pos)
{
    QMenu contextMenu(this);

    QAction* clearAction = contextMenu.addAction("清除所有标注");
    connect(clearAction, &QAction::triggered, this, &ImageGraphicsView::clearAnnotations);

    contextMenu.addSeparator();

    QAction* deleteAction = contextMenu.addAction("删除选中标注");
    connect(deleteAction, &QAction::triggered, this, &ImageGraphicsView::deleteSelectedAnnotation);

    QAction* editAction = contextMenu.addAction("编辑标注属性");
    connect(editAction, &QAction::triggered, this, &ImageGraphicsView::editAnnotationProperties);

    // 检查是否有选中的标注
    QList<QGraphicsItem*> selectedItems = m_graphicsScene->selectedItems();
    bool hasSelection = false;
    for (QGraphicsItem* item : selectedItems) {
        if (dynamic_cast<AnnotationItem*>(item)) {
            hasSelection = true;
            break;
        }
    }

    deleteAction->setEnabled(hasSelection);
    editAction->setEnabled(hasSelection);

    contextMenu.exec(mapToGlobal(pos));
}

void ImageGraphicsView::deleteSelectedAnnotation()
{
    QList<QGraphicsItem*> selectedItems = m_graphicsScene->selectedItems();
    for (QGraphicsItem* item : selectedItems) {
        AnnotationItem* annotation = dynamic_cast<AnnotationItem*>(item);
        if (annotation) {
            m_annotations.removeOne(annotation);
            m_graphicsScene->removeItem(annotation);
            delete annotation;
        }
    }
}

void ImageGraphicsView::editAnnotationProperties()
{
    QList<QGraphicsItem*> selectedItems = m_graphicsScene->selectedItems();
    for (QGraphicsItem* item : selectedItems) {
        AnnotationItem* annotation = dynamic_cast<AnnotationItem*>(item);
        if (annotation) {
            // 这里可以打开属性编辑对话框
            // 暂时简单地改变颜色
            QColor color = QColorDialog::getColor(annotation->getPen().color(), this);
            if (color.isValid()) {
                QPen pen = annotation->getPen();
                pen.setColor(color);
                annotation->setPen(pen);
                annotation->update();
            }
            break;
        }
    }
}

void ImageGraphicsView::scaleView(double scaleFactor)
{
    double newScale = m_scaleFactor * scaleFactor;
    if (newScale < MIN_SCALE || newScale > MAX_SCALE) {
        return;
    }

    scale(scaleFactor, scaleFactor);
    m_scaleFactor = newScale;
    emit scaleChanged(m_scaleFactor);

    // 直接更新信息显示
    updateInfoLabels();
}

void ImageGraphicsView::startAnnotation(const QPointF& scenePos)
{
    if (m_interactionMode != Annotate && m_interactionMode != Measure) {
        return;
    }

    m_currentAnnotation = createAnnotationItem();
    if (m_currentAnnotation) {
        m_currentAnnotation->setStartPoint(scenePos);
        m_currentAnnotation->setEndPoint(scenePos);
        m_currentAnnotation->setPen(m_annotationPen);
        m_currentAnnotation->setBrush(m_annotationBrush);

        m_graphicsScene->addItem(m_currentAnnotation);
        m_isDrawing = true;
    }
}

void ImageGraphicsView::updateAnnotation(const QPointF& scenePos)
{
    if (m_currentAnnotation) {
        m_currentAnnotation->setEndPoint(scenePos);
        m_currentAnnotation->update();
    }
}

void ImageGraphicsView::finishAnnotation(const QPointF& scenePos)
{
    if (m_currentAnnotation) {
        m_currentAnnotation->setEndPoint(scenePos);

        // 检查是否是有效的标注（起点和终点不能太接近）
        QPointF start = m_currentAnnotation->getStartPoint();
        QPointF end = m_currentAnnotation->getEndPoint();
        QPointF diff = end - start;
        double distance = sqrt(diff.x() * diff.x() + diff.y() * diff.y());

        if (distance > 5.0) { // 最小距离阈值
            m_annotations.append(m_currentAnnotation);
            emit annotationAdded(m_currentAnnotation);

            // 如果是测量工具，发出测量信号
            if (m_interactionMode == Measure) {
                MeasurementAnnotation* measurement = dynamic_cast<MeasurementAnnotation*>(m_currentAnnotation);
                if (measurement) {
                    emit measurementTaken(measurement->getDistance(), "px");
                }
            }

            // 对于文本标注，需要获取文本内容
            if (m_annotationTool == TextTool) {
                TextAnnotation* textAnnotation = dynamic_cast<TextAnnotation*>(m_currentAnnotation);
                if (textAnnotation) {
                    bool ok;
                    QString text = QInputDialog::getText(this, "文本标注", "请输入文本:",
                                                        QLineEdit::Normal, "", &ok);
                    if (ok && !text.isEmpty()) {
                        textAnnotation->setText(text);
                    } else {
                        // 用户取消或输入空文本，删除标注
                        m_annotations.removeOne(m_currentAnnotation);
                        m_graphicsScene->removeItem(m_currentAnnotation);
                        delete m_currentAnnotation;
                    }
                }
            }
        } else {
            // 距离太小，删除标注
            m_graphicsScene->removeItem(m_currentAnnotation);
            delete m_currentAnnotation;
        }

        m_currentAnnotation = nullptr;
        m_isDrawing = false;
    }
}

AnnotationItem* ImageGraphicsView::createAnnotationItem()
{
    AnnotationItem* item = nullptr;

    if (m_interactionMode == Measure) {
        item = new MeasurementAnnotation();
    } else {
        switch (m_annotationTool) {
        case LineTool:
            item = new LineAnnotation();
            break;
        case RectangleTool:
            item = new RectangleAnnotation();
            break;
        case EllipseTool:
            item = new EllipseAnnotation();
            break;
        case ArrowTool:
            item = new ArrowAnnotation();
            break;
        case TextTool:
            item = new TextAnnotation();
            break;
        case MeasurementTool:
            item = new MeasurementAnnotation();
            break;
        }
    }

    return item;
}

void ImageGraphicsView::updateInfoLabels()
{
    if (!m_roiOverlay) return;

    // 防止在图像加载过程中更新信息标签
    if (m_isLoadingImage) return;

    QString imageInfo;
    if (!m_currentPixmap.isNull()) {
        imageInfo = QString("Size: %1 x %2 | Zoom: %3%")
                   .arg(m_currentPixmap.width())
                   .arg(m_currentPixmap.height())
                   .arg(qRound(m_scaleFactor * 100));
    } else {
        imageInfo = "No Image";
    }

    // 使用blockSignals防止信号递归
    bool oldState = m_roiOverlay->blockSignals(true);
    m_roiOverlay->setImageInfo(imageInfo);
    m_roiOverlay->blockSignals(oldState);
}

void ImageGraphicsView::updateMousePosition(const QPoint& viewPos)
{
    if (!m_roiOverlay || !scene()) return;

    m_lastMousePos = viewPos;

    // 将视图坐标转换为场景坐标
    QPointF scenePos = mapToScene(viewPos);

    QString mouseInfo;

    if (!m_currentPixmap.isNull()) {
        // 检查鼠标是否在图像范围内
        QRectF imageRect(0, 0, m_currentPixmap.width(), m_currentPixmap.height());

        if (imageRect.contains(scenePos)) {
            int x = qRound(scenePos.x());
            int y = qRound(scenePos.y());

            // 获取像素值
            QImage image = m_currentPixmap.toImage();
            if (x >= 0 && x < image.width() && y >= 0 && y < image.height()) {
                QRgb pixel = image.pixel(x, y);
                int r = qRed(pixel);
                int g = qGreen(pixel);
                int b = qBlue(pixel);

                mouseInfo = QString("Mouse: %1, %2 | RGB: %3, %4, %5")
                           .arg(x).arg(y).arg(r).arg(g).arg(b);
            } else {
                mouseInfo = QString("Mouse: %1, %2 | RGB: ---, ---, ---")
                           .arg(x).arg(y);
            }
        } else {
            mouseInfo = "Mouse: ---, --- | RGB: ---, ---, ---";
        }
    } else {
        mouseInfo = "Mouse: ---, --- | RGB: ---, ---, ---";
    }

    m_roiOverlay->setMouseInfo(mouseInfo);
}

void ImageGraphicsView::paintEvent(QPaintEvent* event)
{
    QGraphicsView::paintEvent(event);

    // 确保ROI蒙层在最上层
    if (m_roiOverlay) {
        m_roiOverlay->raise();
    }
}

void ImageGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    // 调整ROI蒙层大小
    if (m_roiOverlay) {
        m_roiOverlay->resize(size());

        // 重新定位鼠标信息标签到左下角
        int x = 10;
        int y = height() - 30; // 预留足够的空间显示文本
        m_roiOverlay->setMouseInfoPosition(QPoint(x, y));
    }
}

bool ImageGraphicsView::isPointInImage(const QPointF& scenePos) const
{
    if (m_currentPixmap.isNull()) {
        return false;
    }

    // 检查点是否在图像矩形内
    QRectF imageRect(0, 0, m_currentPixmap.width(), m_currentPixmap.height());
    return imageRect.contains(scenePos);
}

void ImageGraphicsView::startROIDrawing(const QPoint& viewPos)
{
    if (!m_roiOverlay) return;

    m_roiOverlay->setDrawing(true);
    m_roiOverlay->resize(size());
    // m_roiOverlay->show();

    // 设置初始矩形
    QRect initialRect(viewPos, viewPos);
    m_roiOverlay->setROIRect(initialRect);
}

void ImageGraphicsView::updateROIDrawing(const QPoint& viewPos)
{
    if (!m_roiOverlay || !m_roiOverlay->isDrawing()) return;

    // 创建矩形（视图坐标系）
    QRect roiRect(m_rightDragStart, viewPos);
    roiRect = roiRect.normalized(); // 确保宽高为正

    m_roiOverlay->setROIRect(roiRect);
}

void ImageGraphicsView::finishROIDrawing()
{
    if (!m_roiOverlay || !m_roiOverlay->isDrawing() || m_currentPixmap.isNull()) return;

    // 获取ROI矩形（视图坐标系）
    QRect viewRect(m_rightDragStart, m_rightDragCurrent);
    viewRect = viewRect.normalized();

    // 确保矩形有效（有面积）
    if (viewRect.width() > 5 && viewRect.height() > 5) {
        // 转换为场景坐标系
        QPointF sceneTopLeft = mapToScene(viewRect.topLeft());
        QPointF sceneBottomRight = mapToScene(viewRect.bottomRight());
        QRectF sceneRect(sceneTopLeft, sceneBottomRight);
        sceneRect = sceneRect.normalized();

        // 确保矩形在图像范围内
        QRectF imageRect(0, 0, m_currentPixmap.width(), m_currentPixmap.height());
        QRectF clampedRect = sceneRect.intersected(imageRect);

        if (!clampedRect.isEmpty()) {
            // 发送信号，传递图像坐标系的矩形
            emit rectangleSelected(clampedRect);
        }
    }

    // 隐藏ROI蒙层
    m_roiOverlay->clearROI();
    // m_roiOverlay->hide();
}

void ImageGraphicsView::calculateAndApplyOptimalWindowLevel(const QRectF& region)
{
    int windowCenter, windowWidth;
    AutoAdjust(m_originPixmap.toImage().copy(region.toRect()).convertToFormat(QImage::Format_Grayscale16), 5000, windowCenter, windowWidth);

    applyWindowLevel(windowCenter, windowWidth);
}

// void ImageGraphicsView::calculateOptimalWindowLevel(const QRectF& region)
// {
//     if (m_originPixmap.isNull() || region.isEmpty()) {
//         return;
//     }

//     // 检查是否有正在进行的计算
//     if (m_windowLevelCalculationWatcher->isRunning()) {
//         return; // 如果有正在进行的计算，则忽略新的请求
//     }

//     // 保存当前计算区域
//     m_currentCalculationRegion = region;

//     // 设置不自动应用标志
//     m_autoApplyAfterCalculation = false;

//     // 发出计算开始信号
//     emit windowLevelCalculationStarted();

//     // 准备计算参数
//     QImage image = m_originPixmap.toImage();
//     WindowLevelCalculator::CalculationParams params(image, region);

//     // 在后台线程中启动计算（仅计算，不自动应用）
//     QFuture<WindowLevelResult> future = QtConcurrent::run([params]() {
//         return WindowLevelCalculator::calculateOptimalWindowLevel(params);
//     });

//     m_windowLevelCalculationWatcher->setFuture(future);
// }



void ImageGraphicsView::applyWindowLevel(double windowCenter, double windowWidth)
{
    QImage image = m_originPixmap.toImage().convertToFormat(QImage::Format_Grayscale16);
    if (image.format() != QImage::Format_Grayscale16) {
        qWarning() << "Image format is not Grayscale16, cannot apply window/level"<<image;
        return;
    }

    // Create output image in 8-bit grayscale format for display
    QImage outputImage(image.width(), image.height(), QImage::Format_Grayscale8);

    // Calculate window bounds
    int windowMin = windowCenter - windowWidth / 2;
    int windowMax = windowCenter + windowWidth / 2;

    // Ensure valid window bounds
    if (windowMax <= windowMin) {
        windowMax = windowMin + 1;
    }

    double scale = 255.0 / (windowMax - windowMin);

    // Apply window/level transformation
    for (int y = 0; y < image.height(); ++y) {
        const quint16* inputLine = reinterpret_cast<const quint16*>(image.constScanLine(y));
        quint8* outputLine = outputImage.scanLine(y);

        for (int x = 0; x < image.width(); ++x) {
            int pixelValue = inputLine[x];

            // Apply window/level transformation
            int outputValue;
            if (pixelValue <= windowMin) {
                outputValue = 0;
            } else if (pixelValue >= windowMax) {
                outputValue = 255;
            } else {
                outputValue = static_cast<int>((pixelValue - windowMin) * scale);
            }

            outputLine[x] = static_cast<quint8>(qBound(0, outputValue, 255));
        }
    }



    m_pixmapItem->setPixmap(QPixmap::fromImage(outputImage));
}

void ImageGraphicsView::onWindowLevelCalculationFinished()
{
    WindowLevelResult result = m_windowLevelCalculationWatcher->result();
    m_lastCalculationResult = result;

    // 发出计算完成信号
    emit windowLevelCalculationFinished(result);

    if (!result.isValid) {
        return; // 计算失败，不继续处理
    }

    // 只有在设置了自动应用标志时才继续应用
    if (!m_autoApplyAfterCalculation) {
        return; // 仅计算模式，不自动应用
    }

    // 检查是否有正在进行的图像应用操作
    if (m_windowLevelApplicationWatcher->isRunning()) {
        return;
    }

    // 在后台线程中应用窗位窗宽到整个图像
    QImage originalImage = m_currentPixmap.toImage();
    double windowLevel = result.windowLevel;
    double windowWidth = result.windowWidth;

    QFuture<QImage> future = QtConcurrent::run([originalImage, windowLevel, windowWidth]() {
        return WindowLevelCalculator::applyWindowLevel(originalImage, windowLevel, windowWidth);
    });

    m_windowLevelApplicationWatcher->setFuture(future);
}

void ImageGraphicsView::onWindowLevelApplicationFinished()
{
    QImage processedImage = m_windowLevelApplicationWatcher->result();

    if (processedImage.isNull()) {
        return; // 处理失败
    }

    // 更新显示的图像
    QPixmap processedPixmap = QPixmap::fromImage(processedImage);

    // 更新场景中的图像
    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(processedPixmap);
    }

    // 更新当前图像
    setCurrentImage(processedPixmap);

    // 发出应用完成信号
    emit windowLevelApplied(m_lastCalculationResult.windowLevel, m_lastCalculationResult.windowWidth);
}

// WindowLevelCalculator 实现
WindowLevelResult WindowLevelCalculator::calculateOptimalWindowLevel(const CalculationParams& params)
{
    if (params.image.isNull() || params.region.isEmpty()) {
        return WindowLevelResult(); // 返回无效结果
    }

    // 提取区域内的像素值
    std::vector<int> pixelValues = extractPixelValues(params.image, params.region);

    if (pixelValues.empty()) {
        return WindowLevelResult(); // 返回无效结果
    }

    // 使用基于直方图的自动阈值算法计算窗位窗宽
    WindowLevelResult result = calculateHistogramBasedWindowLevel(pixelValues, 100); // autoThreshold = 100

    qDebug()<<"WindowLevelResult: "<<result.windowLevel<<", "<<result.windowWidth;

    return result;
}

WindowLevelResult WindowLevelCalculator::calculateHistogramBasedWindowLevel(const std::vector<int>& pixelValues, int autoThreshold)
{
    if (pixelValues.empty()) {
        return WindowLevelResult(); // 返回无效结果
    }

    int pixelCount = pixelValues.size();
    int limit = pixelCount / 10; // 限制异常值的影响

    // 找到最小值和最大值
    auto minMaxPair = std::minmax_element(pixelValues.begin(), pixelValues.end());
    double minVal = *minMaxPair.first;
    double maxVal = *minMaxPair.second;

    if (minVal == maxVal) {
        // 所有像素值相同的情况
        return WindowLevelResult(minVal, 1.0);
    }

    // 创建256个bin的直方图
    std::vector<int> hist(256, 0);
    double binSize = (maxVal - minVal) / 256.0;
    double scale = 256.0 / (maxVal - minVal);

    // 生成直方图
    for (int value : pixelValues) {
        int index = static_cast<int>(scale * (value - minVal));
        index = std::max(0, std::min(255, index)); // 确保索引在有效范围内
        hist[index]++;
    }

    // 使用阈值方法找到有效的灰度范围
    int threshold = pixelCount / autoThreshold;

    // 从左边找到第一个超过阈值的bin
    int hmin = 0;
    for (int i = 0; i < 256; ++i) {
        int count = hist[i];
        if (count > limit) count = 0; // 忽略异常值
        if (count > threshold) {
            hmin = i;
            break;
        }
    }

    // 从右边找到第一个超过阈值的bin
    int hmax = 255;
    for (int i = 255; i >= 0; --i) {
        int count = hist[i];
        if (count > limit) count = 0; // 忽略异常值
        if (count > threshold) {
            hmax = i;
            break;
        }
    }

    // 计算窗位窗宽
    if (hmax >= hmin) {
        double min = minVal + hmin * binSize;
        double max = minVal + hmax * binSize;

        if (min == max) {
            min = minVal;
            max = maxVal;
        }

        double windowWidth = max - min;
        double windowLevel = (max + min) / 2.0;

        // 确保窗宽不会太小
        if (windowWidth < 1.0) {
            windowWidth = 1.0;
        }

        return WindowLevelResult(windowLevel, windowWidth);
    }

    // 如果算法失败，回退到简单的最小最大值方法
    return WindowLevelResult((minVal + maxVal) / 2.0, maxVal - minVal);
}

QImage WindowLevelCalculator::applyWindowLevel(const QImage& image, double windowLevel, double windowWidth)
{
    qDebug()<<"applyWindowLevel: "<<windowLevel<<", "<<windowWidth;
    if (image.isNull() || windowWidth <= 0) {
        return image;
    }

    QImage result = image.convertToFormat(QImage::Format_RGB32);

    double minValue = windowLevel - windowWidth / 2.0;
    double maxValue = windowLevel + windowWidth / 2.0;
    double range = maxValue - minValue;

    if (range <= 0) {
        return result;
    }

    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            // 转换为灰度值（使用亮度公式）
            double gray = 0.299 * qRed(pixel) + 0.587 * qGreen(pixel) + 0.114 * qBlue(pixel);

            // 应用窗位窗宽
            double normalized = (gray - minValue) / range;
            normalized = std::max(0.0, std::min(1.0, normalized)); // 限制在 [0, 1] 范围内

            int newValue = static_cast<int>(normalized * 255);
            line[x] = qRgb(newValue, newValue, newValue);
        }
    }

    return result;
}

std::vector<int> WindowLevelCalculator::extractPixelValues(const QImage& image, const QRectF& region)
{
    std::vector<int> values;

    // 确保区域在图像范围内
    QRectF imageRect(0, 0, image.width(), image.height());
    QRectF clampedRegion = region.intersected(imageRect);

    if (clampedRegion.isEmpty()) {
        return values;
    }

    // 修复边界计算，确保不会越界
    int startX = static_cast<int>(std::max(0.0, clampedRegion.left()));
    int endX = static_cast<int>(std::min(static_cast<double>(image.width() - 1),
                                       std::floor(clampedRegion.right() - 1)));
    int startY = static_cast<int>(std::max(0.0, clampedRegion.top()));
    int endY = static_cast<int>(std::min(static_cast<double>(image.height() - 1),
                                       std::floor(clampedRegion.bottom() - 1)));

    // 确保有效范围
    if (endX < startX || endY < startY) {
        return values;
    }

    values.reserve((endX - startX + 1) * (endY - startY + 1));

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            QRgb pixel = image.pixel(x, y);
            // 转换为灰度值
            int gray = static_cast<int>(0.299 * qRed(pixel) + 0.587 * qGreen(pixel) + 0.114 * qBlue(pixel));
            values.push_back(gray);
        }
    }

    return values;
}

double WindowLevelCalculator::calculateMean(const std::vector<int>& values)
{
    if (values.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (int value : values) {
        sum += value;
    }

    return sum / values.size();
}

double WindowLevelCalculator::calculateStandardDeviation(const std::vector<int>& values, double mean)
{
    if (values.size() <= 1) {
        return 0.0;
    }

    double sumSquaredDiff = 0.0;
    for (int value : values) {
        double diff = value - mean;
        sumSquaredDiff += diff * diff;
    }

    return std::sqrt(sumSquaredDiff / (values.size() - 1));
}

double WindowLevelCalculator::calculatePercentile(std::vector<int> values, double percentile)
{
    if (values.empty()) {
        return 0.0;
    }

    if (values.size() == 1) {
        return values[0];
    }

    // 确保已排序
    std::sort(values.begin(), values.end());

    double index = (percentile / 100.0) * (values.size() - 1);
    int lowerIndex = static_cast<int>(std::floor(index));
    int upperIndex = static_cast<int>(std::ceil(index));

    if (lowerIndex == upperIndex) {
        return values[lowerIndex];
    }

    double weight = index - lowerIndex;
    return values[lowerIndex] * (1.0 - weight) + values[upperIndex] * weight;
}

// #include "imagegraphicsview.moc"
