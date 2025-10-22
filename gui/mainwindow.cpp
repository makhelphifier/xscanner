#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QTimer>
#include <QToolBar>
#include <QDebug>
#include "annotationlineitem.h"
#include "annotationrectitem.h"
#include "annotationellipseitem.h"
#include "annotationpointitem.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QFileDialog>
#include <QMenuBar>
#include "gui/imageviewer.h"
#include <QLabel>
#include "gui/toprightinfowidget.h"
#include "service/imageprocessor.h"
#include <QActionGroup>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(800, 600);
    viewer = new ImageViewer(this);
    setCentralWidget(viewer);

    // --- 添加尺寸标签 ---
    sizeLabel = new QLabel(viewer);
    sizeLabel->setStyleSheet(
        "background-color: rgba(0, 0, 0, 150);"
        "color: white;"
        "padding: 4px;"
        "border-radius: 4px;"
        );
    sizeLabel->move(10, 10);
    sizeLabel->setVisible(false);

    // --- 添加坐标和灰度值标签 ---
    infoLabel = new QLabel(viewer);
    infoLabel->setStyleSheet(
        "background-color: rgba(0, 0, 0, 150);"
        "color: white;"
        "padding: 4px;"
        "border-radius: 4px;"
        );
    infoLabel->setText("X: N/A, Y: N/A, value: N/A");
    infoLabel->adjustSize(); // 根据内容调整初始大小
    infoLabel->move(10, viewer->height() - infoLabel->height() - 10); // 定位到左下角
    infoLabel->setVisible(true); // 始终可见
    // -------------------------

    // --- 添加右上角信息控件 ---
    infoWidget = new TopRightInfoWidget(viewer);
    infoWidget->setVisible(false); // 初始时隐藏
    // -------------------------


    // 安装事件过滤器
    viewer->viewport()->installEventFilter(this);
    qDebug() << "Event filter installed on viewer:" << viewer;

    // 启用鼠标跟踪
    viewer->setMouseTracking(true);
    viewer->viewport()->setMouseTracking(true);

    // 设置交互模式
    viewer->setInteractive(true);
    viewer->setDragMode(QGraphicsView::ScrollHandDrag);

    // 添加菜单
    QMenu *fileMenu = menuBar()->addMenu("文件");
    openAction = fileMenu->addAction("打开");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    QToolBar *toolBar = new QToolBar("测量工具", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);
    toolGroup = new QActionGroup(this);

    // 添加选择工具
    selectAction = new QAction(QIcon(":/Resources/img/arrow_tool.png"), "选择/拖动", this);
    selectAction->setCheckable(true);
    toolBar->addAction(selectAction);
    toolGroup->addAction(selectAction);
    connect(selectAction, &QAction::triggered, this, &MainWindow::selectMode);

    // 添加窗宽窗位工具
    wlAction = new QAction(QIcon(":/Resources/img/u27.png"), "窗宽窗位", this); // 使用一个图标
    wlAction->setCheckable(true);
    toolBar->addAction(wlAction);
    toolGroup->addAction(wlAction);
    connect(wlAction, &QAction::triggered, this, &MainWindow::selectWindowLevel);


    lineAction = new QAction(QIcon(":/Resources/img/line_tool.png"), "直线", this);
    lineAction->setCheckable(true);
    toolBar->addAction(lineAction);
    toolGroup->addAction(lineAction);
    connect(lineAction, &QAction::triggered, this, &MainWindow::drawLine);

    rectAction = new QAction(QIcon(":/Resources/img/rect_tool.png"), "矩形", this);
    rectAction->setCheckable(true);
    toolBar->addAction(rectAction);
    toolGroup->addAction(rectAction);
    connect(rectAction, &QAction::triggered, this, &MainWindow::drawRect);

    ellipseAction = new QAction(QIcon(":/Resources/img/ellipse_tool.png"), "椭圆", this);
    ellipseAction->setCheckable(true);
    toolBar->addAction(ellipseAction);
    toolGroup->addAction(ellipseAction);
    connect(ellipseAction, &QAction::triggered, this, &MainWindow::drawEllipse);

    // --- 连接信号和槽 ---
    connect(viewer, &ImageViewer::scaleChanged, this, &MainWindow::updateScale);
    connect(infoWidget, &TopRightInfoWidget::scaleEdited, this, &MainWindow::onScaleFromWidget);
    connect(infoWidget, &TopRightInfoWidget::autoWindowingToggled, this, &MainWindow::onAutoWindowingToggled);
    connect(infoWidget, &TopRightInfoWidget::windowChanged, this, &MainWindow::onWindowChanged); // <-- 添加此行
    connect(infoWidget, &TopRightInfoWidget::levelChanged, this, &MainWindow::onLevelChanged);   // <-- 添加此行

    selectAction->setChecked(true);
    selectMode();

    // 默认加载图像
    QString defaultPath = ":/Resources/img/000006.raw";
    QImage loadedImage;
    if (defaultPath.endsWith(".raw", Qt::CaseInsensitive) || defaultPath.endsWith(".bin", Qt::CaseInsensitive)) {
        // 加载16位RAW图像（不归一化），假设尺寸为 2882x2340（根据文件调整如果需要）
        loadedImage = ImageProcessor::readRawImg_qImage(defaultPath, 2882, 2340);
    } else {
        // 加载标准图像文件
        loadedImage.load(defaultPath);
    }

    if (!loadedImage.isNull()) {
        // 检测位深
        m_bitDepth = (loadedImage.format() == QImage::Format_Grayscale16) ? 16 : 8;

        // 如果是RGB图像，转换为灰度（假设为8位）
        if (!loadedImage.isGrayscale()) {
            loadedImage = loadedImage.convertToFormat(QImage::Format_Grayscale8);
            m_bitDepth = 8;
        }

        // 统一处理加载成功的图像
        m_originalImage = loadedImage;
        viewer->setImage(m_originalImage);

        sizeLabel->setText(QString("size: %1x%2").arg(m_originalImage.width()).arg(m_originalImage.height()));
        sizeLabel->adjustSize();
        sizeLabel->setVisible(true);

        infoWidget->setVisible(true);
        // 根据位深设置窗宽窗位滑动条范围
        int maxVal = (m_bitDepth == 16) ? 65535 : 255;
        infoWidget->setWindowRange(1, maxVal + 1);  // 窗宽范围：1 到 maxVal+1
        infoWidget->setLevelRange(0, maxVal);       // 窗位范围：0 到 maxVal

        // 初始化为全范围（手动调节默认值）
        m_windowWidth = maxVal + 1;
        m_windowLevel = maxVal / 2;
        infoWidget->setWindowValue(m_windowWidth);
        infoWidget->setLevelValue(m_windowLevel);
        applyAndDisplayWl();  // 应用初始窗位窗宽
        infoWidget->setAutoWindowingChecked(true);

    }

    // 手动调用一次 resizeEvent 来正确设置初始位置
    QTimer::singleShot(0, this, [this](){
        resizeEvent(nullptr);
    });
}

MainWindow::~MainWindow() {}


void MainWindow::openImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "打开图片", "", "Raw Binary (*.raw *.bin);;Images (*.png *.jpg *.bmp)");

    if (filePath.isEmpty()) {
        return;
    }

    qDebug() << "Selected file path:" << filePath;

    QImage loadedImage;
    if (filePath.endsWith(".raw", Qt::CaseInsensitive) || filePath.endsWith(".bin", Qt::CaseInsensitive)) {
        // 加载16位RAW图像（不归一化）
        loadedImage = ImageProcessor::readRawImg_qImage(filePath, 2882, 2340);
    } else {
        // 加载标准图像文件（可能为8位或16位，取决于文件）
        loadedImage.load(filePath);
    }

    if (loadedImage.isNull()) {
        qDebug() << "Failed to process image from path:" << filePath;
        m_originalImage = QImage();
        viewer->setImage(QImage()); // 清空视图
        sizeLabel->setVisible(false);
        infoWidget->setVisible(false);
        return;
    }

    // 检测位深
    m_bitDepth = (loadedImage.format() == QImage::Format_Grayscale16) ? 16 : 8;

    // 如果是RGB图像，转换为灰度（假设为8位）
    if (!loadedImage.isGrayscale()) {
        loadedImage = loadedImage.convertToFormat(QImage::Format_Grayscale8);
        m_bitDepth = 8;
    }

    // 统一处理加载成功的图像
    m_originalImage = loadedImage;
    viewer->setImage(m_originalImage);

    sizeLabel->setText(QString("size: %1x%2").arg(m_originalImage.width()).arg(m_originalImage.height()));
    sizeLabel->adjustSize();
    sizeLabel->setVisible(true);

    infoWidget->setVisible(true);
    // 根据位深设置窗宽窗位滑动条范围
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    infoWidget->setWindowRange(1, maxVal + 1);  // 窗宽范围：1 到 maxVal+1
    infoWidget->setLevelRange(0, maxVal);       // 窗位范围：0 到 maxVal

    // 初始化为全范围（手动调节默认值）
    m_windowWidth = maxVal + 1;
    m_windowLevel = maxVal / 2;
    infoWidget->setWindowValue(m_windowWidth);
    infoWidget->setLevelValue(m_windowLevel);
    applyAndDisplayWl();  // 应用初始窗位窗宽
    infoWidget->setAutoWindowingChecked(true);

    // 默认执行一次自动窗宽窗位
    onAutoWindowingToggled(true);
    infoWidget->checkAutoWindowing(); // 确保UI同步

}

void MainWindow::resizeEvent(QResizeEvent *event)
{

    QMainWindow::resizeEvent(event);
    // --- 窗口大小改变时，重新定位左下角的标签 ---
    if (infoLabel) {
        infoLabel->move(10, viewer->height() - infoLabel->height() - 10);
    }
    // --- 重新定位右上角控件 ---
    if (infoWidget) {
        infoWidget->move(viewer->width() - infoWidget->width() - 10, 10);
    }
}
void MainWindow::drawLine()
{
    m_currentMode = Mode_Line;
    m_isDrawing = false;
    qDebug() << "drawLine called, mode:" << m_currentMode << "drawing:" << m_isDrawing;
    viewer->setDragMode(QGraphicsView::NoDrag);
    viewer->viewport()->setCursor(Qt::CrossCursor);
}

void MainWindow::drawRect()
{
    m_currentMode = Mode_Rect;
    viewer->setDragMode(QGraphicsView::NoDrag);
    viewer->viewport()->setCursor(Qt::CrossCursor);

}

void MainWindow::drawEllipse()
{
    m_currentMode = Mode_Ellipse;
    viewer->setDragMode(QGraphicsView::NoDrag);
    viewer->viewport()->setCursor(Qt::CrossCursor);
}

void MainWindow::drawPoint()
{
    m_currentMode = Mode_Point;
    viewer->setDragMode(QGraphicsView::NoDrag);
    viewer->viewport()->setCursor(Qt::CrossCursor);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != viewer->viewport() || m_originalImage.isNull()) {
        return QObject::eventFilter(watched, event);  // 基类处理（QMainWindow 继承自 QObject）
    }

    switch (event->type()) {
    case QEvent::MouseMove:
        return handleMouseMove(static_cast<QMouseEvent*>(event));
    case QEvent::MouseButtonPress:
        return handleMousePress(static_cast<QMouseEvent*>(event));
    case QEvent::MouseButtonRelease:
        return handleMouseRelease(static_cast<QMouseEvent*>(event));
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

// 处理鼠标移动：更新标签和预览
bool MainWindow::handleMouseMove(QMouseEvent* mouseEvent)
{
    QPointF scenePos = viewer->mapToScene(mouseEvent->pos());
    updateInfoLabel(scenePos);

    // 更新预览（仅在绘制中）
    if (m_currentMode == Mode_Line && m_isDrawing && m_previewLine) {
        m_previewLine->setLine(QLineF(m_startPoint, scenePos));
        return true;
    } else if (m_currentMode == Mode_WindowLevel && m_isDrawing && m_previewRect) {
        m_previewRect->setRect(QRectF(m_startPoint, scenePos).normalized());
        return true;
    }

    return false;  // 未消费事件
}

// 处理鼠标按下：开始绘制
bool MainWindow::handleMousePress(QMouseEvent* mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton) {
        return false;
    }

    QPointF scenePos = viewer->mapToScene(mouseEvent->pos());

    switch (m_currentMode) {
    case Mode_Line:
        if (!m_isDrawing) {
            m_startPoint = scenePos;
            m_isDrawing = true;
            m_previewLine = createPreviewLine(m_startPoint);
            qDebug() << "Started drawing line at:" << m_startPoint;
        }
        return true;

    case Mode_WindowLevel:
        m_startPoint = scenePos;
        m_isDrawing = true;
        m_previewRect = createPreviewRect(m_startPoint);
        return true;

        // TODO: 如果需要，为 Mode_Rect, Mode_Ellipse, Mode_Point 添加类似逻辑
    default:
        return false;
    }
}

// 处理鼠标释放：完成绘制
bool MainWindow::handleMouseRelease(QMouseEvent* mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton || !m_isDrawing) {
        return false;
    }

    QPointF scenePos = viewer->mapToScene(mouseEvent->pos());

    switch (m_currentMode) {
    case Mode_Line:
        if (m_previewLine) {
            // 移除预览
            viewer->scene()->removeItem(m_previewLine);
            delete m_previewLine;
            m_previewLine = nullptr;
        }
        finishDrawingLine(scenePos);
        return true;

    case Mode_WindowLevel:
        if (m_previewRect) {
            QRectF rect = m_previewRect->rect();
            viewer->scene()->removeItem(m_previewRect);
            delete m_previewRect;
            m_previewRect = nullptr;
            finishWindowLevelRect(rect);
        }
        return true;

        // TODO: 如果需要，为其他模式添加释放逻辑
    default:
        return false;
    }
}

// 更新坐标和灰度值标签
void MainWindow::updateInfoLabel(const QPointF& scenePos)
{
    QString infoText;
    if (viewer->pixmapItem() && viewer->pixmapItem()->boundingRect().contains(scenePos)) {
        int x = qRound(scenePos.x());
        int y = qRound(scenePos.y());
        if (m_originalImage.valid(x, y)) {
            int grayValue = getPixelValue(x, y);
            infoText = QString("X: %1, Y: %2, value: %3").arg(x).arg(y).arg(grayValue);
        } else {
            infoText = QString("X: %1, Y: %2, value: N/A").arg(x).arg(y);
        }
    } else {
        infoText = QString("X: %1, Y: %2, value: N/A")
                       .arg(qRound(scenePos.x()))
                       .arg(qRound(scenePos.y()));
    }
    infoLabel->setText(infoText);
    infoLabel->adjustSize();
}

// 获取像素值（提取重复逻辑）
int MainWindow::getPixelValue(int x, int y) const
{
    if (m_bitDepth == 16) {
        const quint16* scanLine = reinterpret_cast<const quint16*>(m_originalImage.constScanLine(y));
        return scanLine[x];
    } else {
        return qGray(m_originalImage.pixel(x, y));
    }
}

// 创建线预览项（提取公共代码）
QGraphicsLineItem* MainWindow::createPreviewLine(const QPointF& start)
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
    viewer->scene()->addItem(preview);
    return preview;
}

// 创建矩形预览项（提取公共代码）
QGraphicsRectItem* MainWindow::createPreviewRect(const QPointF& start)
{
    QGraphicsRectItem* preview = new QGraphicsRectItem(QRectF(start, start));
    QPen pen(Qt::yellow, 1, Qt::DashLine);
    preview->setPen(pen);
    viewer->scene()->addItem(preview);
    return preview;
}

// 完成线绘制
void MainWindow::finishDrawingLine(const QPointF& endPoint)
{
    AnnotationLineItem* line = new AnnotationLineItem(
        m_startPoint.x(), m_startPoint.y(),
        endPoint.x(), endPoint.y()
        );
    viewer->scene()->addItem(line);
    m_isDrawing = false;
    switchToSelectMode();
    qDebug() << "Line completed from" << m_startPoint << "to" << endPoint;
}

// 完成窗宽窗位矩形（优化 min/max 计算）
void MainWindow::finishWindowLevelRect(const QRectF& rect)
{
    if (!rect.isValid()) {
        m_isDrawing = false;
        switchToSelectMode();
        return;
    }

    // 计算区域 min/max（优化：使用边界整数化，避免浮点）
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
                int grayValue = getPixelValue(x, y);  // 重用方法
                minVal = qMin(minVal, grayValue);
                maxVal = qMax(maxVal, grayValue);
            }
        }
    }

    if (foundPixel && minVal < maxVal) {
        m_windowWidth = maxVal - minVal;
        m_windowLevel = minVal + m_windowWidth / 2;
        infoWidget->setWindowValue(m_windowWidth);
        infoWidget->setLevelValue(m_windowLevel);
        infoWidget->uncheckAutoWindowing();
        applyAndDisplayWl();
    }

    m_isDrawing = false;
    switchToSelectMode();
}

// 切换回选择模式（提取公共代码）
void MainWindow::switchToSelectMode()
{
    selectAction->setChecked(true);
    selectMode();
}



void MainWindow::selectMode()
{
    m_currentMode = Mode_Select;
    m_isDrawing = false;
    viewer->setDragMode(QGraphicsView::ScrollHandDrag);
    viewer->viewport()->setCursor(Qt::OpenHandCursor);
}


void MainWindow::updateScale(qreal scale)
{
    if (infoWidget) {
        infoWidget->setScale(scale);
    }
}

void MainWindow::onScaleFromWidget(double scale)
{
    if (viewer) {
        viewer->setScale(scale);
    }
}

void MainWindow::onAutoWindowingToggled(bool checked)
{
    if (m_originalImage.isNull()) return;

    int maxVal = (m_bitDepth == 16) ? 65535 : 255;

    if (checked) {
        int min, max;
        ImageProcessor::calculateAutoWindowLevel(m_originalImage, min, max);

        // 更新成员变量
        m_windowWidth = max - min;
        m_windowLevel = min + m_windowWidth / 2;

        applyAndDisplayWl();
    } else {
        // 恢复到完整的范围
        m_windowWidth = maxVal + 1;
        m_windowLevel = maxVal / 2;
        applyAndDisplayWl();
    }

    // 更新滑动条位置和图像
    infoWidget->setWindowValue(m_windowWidth);
    infoWidget->setLevelValue(m_windowLevel);
    applyAndDisplayWl();
}


void MainWindow::onWindowChanged(int value)
{
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    m_windowWidth = qBound(1, value, maxVal + 1);  // 夹紧窗宽到有效范围
    infoWidget->uncheckAutoWindowing();  // 手动调整时取消自动模式
    applyAndDisplayWl();
}

void MainWindow::onLevelChanged(int value)
{
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    m_windowLevel = qBound(0, value, maxVal);  // 夹紧窗位到有效范围
    infoWidget->uncheckAutoWindowing();  // 手动调整时取消自动模式
    applyAndDisplayWl();
}


void MainWindow::applyAndDisplayWl()
{
    if (m_originalImage.isNull()) return;

    // 根据窗宽窗位计算 min 和 max
    int min = m_windowLevel - m_windowWidth / 2;
    int max = m_windowLevel + m_windowWidth / 2;

    // 夹紧 min 和 max 到图像值范围
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    min = qBound(0, min, maxVal);
    max = qBound(0, max, maxVal);
    if (min > max) std::swap(min, max);

    // 应用并获取新图像
    QImage adjustedImage = ImageProcessor::applyWindowLevel(m_originalImage, min, max);

    // 更新显示
    viewer->updatePixmap(QPixmap::fromImage(adjustedImage));
    infoWidget->setWindowLevelText(QString("window/level: %1/%2").arg(m_windowWidth).arg(m_windowLevel));
}


void MainWindow::selectWindowLevel()
{
    m_currentMode = Mode_WindowLevel;
    m_isDrawing = false;
    viewer->setDragMode(QGraphicsView::NoDrag);
    viewer->viewport()->setCursor(Qt::CrossCursor);
}


