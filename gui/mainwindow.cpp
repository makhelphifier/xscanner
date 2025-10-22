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
#include <QFileDialog> // 确保这个 include 存在


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
        // 修改下面这行来加载16位图像
        loadedImage = ImageProcessor::readRawImg_qImage(filePath, 2882, 2340);
    } else {
        // 加载标准图像文件
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

    // 统一处理加载成功的图像
    m_originalImage = loadedImage;
    viewer->setImage(m_originalImage);

    sizeLabel->setText(QString("size: %1x%2").arg(m_originalImage.width()).arg(m_originalImage.height()));
    sizeLabel->adjustSize();
    sizeLabel->setVisible(true);

    infoWidget->setVisible(true);
    // 重置窗宽窗位设置
    // onAutoWindowingToggled(false);
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
    if (watched == viewer->viewport()) {
        // qDebug() << "Viewer event:" << event->type();
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);



            QPointF scenePos = viewer->mapToScene(mouseEvent->pos());

            // --- 更新坐标和灰度值 ---
            QString infoText;
            // 检查鼠标是否在图片范围内
            if (viewer->pixmapItem() && viewer->pixmapItem()->boundingRect().contains(scenePos)) {
                int x = qRound(scenePos.x());
                int y = qRound(scenePos.y());
                if (m_originalImage.valid(x, y)) {
                    int grayValue = qGray(m_originalImage.pixel(x, y));
                    infoText = QString("X: %1, Y: %2, value: %3")
                                   .arg(x)
                                   .arg(y)
                                   .arg(grayValue);
                } else {
                    infoText = QString("X: %1, Y: %2, value: N/A")
                                   .arg(x)
                                   .arg(y);
                }
            } else {
                infoText = QString("X: %1, Y: %2, value: N/A")
                               .arg(qRound(scenePos.x()))
                               .arg(qRound(scenePos.y()));
            }
            infoLabel->setText(infoText);
            infoLabel->adjustSize();

            // qDebug() << "MouseMove - mode:" << m_currentMode
            //          << "drawing:" << m_isDrawing
            //          << "previewLine:" << (m_previewLine != nullptr)
            //          << "scenePos:" << scenePos;

            if (m_currentMode == Mode_Line && m_isDrawing && m_previewLine) {
                m_previewLine->setLine(QLineF(m_startPoint, scenePos));
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF scenePos = viewer->mapToScene(mouseEvent->pos());
            if (mouseEvent->button() == Qt::LeftButton && m_currentMode == Mode_Line) {
                if (!m_isDrawing) {
                    m_startPoint = scenePos;
                    m_isDrawing = true;
                    if (m_previewLine) {
                        viewer->scene()->removeItem(m_previewLine);
                        delete m_previewLine;
                    }
                    m_previewLine = new QGraphicsLineItem(QLineF(m_startPoint, m_startPoint));
                    QPen pen(Qt::yellow);
                    pen.setStyle(Qt::DashLine);
                    pen.setWidthF(2.0);
                    m_previewLine->setPen(pen);
                    m_previewLine->setZValue(1000);
                    m_previewLine->setFlag(QGraphicsItem::ItemIsSelectable, false);
                    m_previewLine->setFlag(QGraphicsItem::ItemIsMovable, false);
                    m_previewLine->setAcceptHoverEvents(false);
                    viewer->scene()->addItem(m_previewLine);
                    qDebug() << "Started drawing line at:" << m_startPoint;
                } else {
                    if (m_previewLine) {
                        viewer->scene()->removeItem(m_previewLine);
                        delete m_previewLine;
                        m_previewLine = nullptr;
                    }
                    AnnotationLineItem *line = new AnnotationLineItem(
                        m_startPoint.x(), m_startPoint.y(),
                        scenePos.x(), scenePos.y()
                        );
                    viewer->scene()->addItem(line);
                    m_isDrawing = false;
                    // 完成后自动切回选择模式
                    selectAction->setChecked(true);
                    selectMode();
                    qDebug() << "Line completed from" << m_startPoint << "to" << scenePos;
                }
                return true;
            }
            else if (event->type() == QEvent::MouseButtonRelease) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    if (m_currentMode == Mode_Select) {
                        viewer->viewport()->setCursor(Qt::OpenHandCursor);
                    }
                }
            }
        }

    }
    return QMainWindow::eventFilter(watched, event);
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

    if (checked) {
        int min, max;
        ImageProcessor::calculateAutoWindowLevel(m_originalImage, min, max);

        // 更新成员变量
        m_windowWidth = max - min;
        m_windowLevel = min + m_windowWidth / 2;

        applyAndDisplayWl();
    } else {
        // 恢复到完整的范围
        m_windowWidth = 256;
        m_windowLevel = 128;
        applyAndDisplayWl();
    }

    // 更新滑动条位置和图像
    infoWidget->setWindowValue(m_windowWidth);
    infoWidget->setLevelValue(m_windowLevel);
    applyAndDisplayWl();
}



void MainWindow::onWindowChanged(int value)
{
    m_windowWidth = value;
    infoWidget->uncheckAutoWindowing(); // 手动调整时取消自动模式
    applyAndDisplayWl();
}

void MainWindow::onLevelChanged(int value)
{
    m_windowLevel = value;
    infoWidget->uncheckAutoWindowing(); // 手动调整时取消自动模式
    applyAndDisplayWl();
}


// 辅助函数，用于应用当前的窗宽窗位并更新显示
void MainWindow::applyAndDisplayWl()
{
    if (m_originalImage.isNull()) return;

    // 根据窗宽窗位计算 min 和 max
    int min = m_windowLevel - m_windowWidth / 2;
    int max = m_windowLevel + m_windowWidth / 2;

    // 应用并获取新图像
    QImage adjustedImage = ImageProcessor::applyWindowLevel(m_originalImage, min, max);

    // 更新显示
    viewer->updatePixmap(QPixmap::fromImage(adjustedImage));
    infoWidget->setWindowLevelText(QString("window/level: %1/%2").arg(m_windowWidth).arg(m_windowLevel));
}

