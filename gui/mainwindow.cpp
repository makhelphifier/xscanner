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
#include "gui/toprightinfowidget.h" // <-- 添加此行




MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(800, 600);
    viewer = new ImageViewer(this);
    setCentralWidget(viewer);

    // --- 添加尺寸标签 ---
    sizeLabel = new QLabel(viewer); // <-- 将 viewer 作为父控件
    sizeLabel->setStyleSheet(
        "background-color: rgba(0, 0, 0, 150);"
        "color: white;"
        "padding: 4px;"
        "border-radius: 4px;"
        );
    sizeLabel->move(10, 10); // <-- 设置初始位置
    sizeLabel->setVisible(false); // <-- 初始时隐藏
    // --------------------

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

    QString defaultImagePath = ":/Resources/img/aaa.jpg";
    viewer->loadImage(defaultImagePath);

    // --- 为默认图片更新尺寸 ---
    QPixmap pixmap(defaultImagePath);
    if (!pixmap.isNull()) {
        m_image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        sizeLabel->setText(QString("size: %1x%2").arg(pixmap.width()).arg(pixmap.height()));
        sizeLabel->adjustSize(); // 根据内容调整大小
        sizeLabel->setVisible(true);
        infoWidget->setVisible(true); // 加载成功后显示
    }

    // --- 连接信号和槽 ---
    connect(viewer, &ImageViewer::scaleChanged, this, &MainWindow::updateScale);
    connect(infoWidget, &TopRightInfoWidget::scaleEdited, this, &MainWindow::onScaleFromWidget);
    // -------------------


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
    QString filePath = QFileDialog::getOpenFileName(this, "打开图片", "", "Images (*.png *.jpg *.bmp)");
    if (!filePath.isEmpty()) {
        qDebug() << "Selected image path:" << filePath;
        viewer->loadImage(filePath);
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            m_image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
            // --- 更新尺寸标签 ---
            sizeLabel->setText(QString("size: %1x%2").arg(pixmap.width()).arg(pixmap.height()));
            sizeLabel->adjustSize(); // 根据内容调整大小
            sizeLabel->setVisible(true);
            infoWidget->setVisible(true); // <-- 添加此行
            // ---------------------
        } else {
            m_image = QImage();
            sizeLabel->setVisible(false); // 加载失败则隐藏
            infoWidget->setVisible(false); // <-- 添加此行
        }
    } else {
        qDebug() << "No image file selected.";
    }
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
                if (m_image.valid(x, y)) {
                    int grayValue = qGray(m_image.pixel(x, y));
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
            // -------------------------

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


void MainWindow::updateScale(qreal scale) // <-- 添加整个函数
{
    if (infoWidget) {
        infoWidget->setScale(scale);
    }
}

void MainWindow::onScaleFromWidget(double scale) // <-- 添加整个函数
{
    if (viewer) {
        viewer->setScale(scale);
    }
}
