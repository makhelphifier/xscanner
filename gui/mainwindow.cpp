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
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(800, 600);
    viewer = new ImageViewer(this);
    setCentralWidget(viewer);

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
    toolGroup = new QActionGroup(this); // <-- 添加此行

    // 添加选择工具
    selectAction = new QAction(QIcon(":/Resources/img/arrow_tool.png"), "选择/拖动", this);
    selectAction->setCheckable(true); // <-- 添加此行
    toolBar->addAction(selectAction);
    toolGroup->addAction(selectAction); // <-- 添加此行
    connect(selectAction, &QAction::triggered, this, &MainWindow::selectMode);

    lineAction = new QAction(QIcon(":/Resources/img/line_tool.png"), "直线", this);
    lineAction->setCheckable(true); // <-- 添加此行
    toolBar->addAction(lineAction);
    toolGroup->addAction(lineAction); // <-- 添加此行
    connect(lineAction, &QAction::triggered, this, &MainWindow::drawLine);

    rectAction = new QAction(QIcon(":/Resources/img/rect_tool.png"), "矩形", this);
    rectAction->setCheckable(true); // <-- 添加此行
    toolBar->addAction(rectAction);
    toolGroup->addAction(rectAction); // <-- 添加此行
    connect(rectAction, &QAction::triggered, this, &MainWindow::drawRect);

    ellipseAction = new QAction(QIcon(":/Resources/img/ellipse_tool.png"), "椭圆", this);
    ellipseAction->setCheckable(true); // <-- 添加此行
    toolBar->addAction(ellipseAction);
    toolGroup->addAction(ellipseAction); // <-- 添加此行
    connect(ellipseAction, &QAction::triggered, this, &MainWindow::drawEllipse);

    QString defaultImagePath = ":/Resources/img/aaa.jpg";
    viewer->loadImage(defaultImagePath);

    selectAction->setChecked(true); // <-- 添加此行, 默认选中
    selectMode(); // <-- 添加此行, 初始化模式
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
        } else {
            m_image = QImage();
        }
    } else {
        qDebug() << "No image file selected.";
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

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
        qDebug() << "Viewer event:" << event->type();
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF scenePos = viewer->mapToScene(mouseEvent->pos());
            qDebug() << "MouseMove - mode:" << m_currentMode
                     << "drawing:" << m_isDrawing
                     << "previewLine:" << (m_previewLine != nullptr)
                     << "scenePos:" << scenePos;
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
                    selectAction->setChecked(true); // <-- 添加此行
                    selectMode();                   // <-- 添加此行
                    viewer->viewport()->setCursor(Qt::ArrowCursor);
                    qDebug() << "Line completed from" << m_startPoint << "to" << scenePos;
                }
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}


void MainWindow::selectMode() // <-- 添加整个函数
{
    m_currentMode = Mode_Select;
    m_isDrawing = false;
    viewer->setDragMode(QGraphicsView::ScrollHandDrag);
    viewer->viewport()->setCursor(Qt::OpenHandCursor);
}
