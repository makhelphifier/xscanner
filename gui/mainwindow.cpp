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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    resize(800,600);
    viewer = new ImageViewer(this);
    setCentralWidget(viewer);
    viewer->installEventFilter(this);
    // 添加菜单
    QMenu *fileMenu = menuBar()->addMenu("文件");
    openAction = fileMenu->addAction("打开");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    QToolBar *toolBar = new QToolBar("测量工具", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);


    lineAction = new QAction(QIcon(":/Resources/img/line_tool.png"),"直线", this);
    toolBar->addAction(lineAction);
    connect(lineAction, &QAction::triggered, this, &MainWindow::drawLine);

    rectAction = new QAction(QIcon(":/Resources/img/rect_tool.png"),"矩形", this);
    toolBar->addAction(rectAction);
    connect(rectAction, &QAction::triggered, this, &MainWindow::drawRect);

    ellipseAction = new QAction(QIcon(":/Resources/img/ellipse_tool.png"),"椭圆", this);
    toolBar->addAction(ellipseAction);
    connect(ellipseAction, &QAction::triggered, this, &MainWindow::drawEllipse);
    // QString defaultImagePath = QCoreApplication::applicationDirPath() + "/image/aaa.jpg";
    QString defaultImagePath =  ":/Resources/img/bbb.jpg";
    viewer->loadImage(defaultImagePath);
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
    viewer->setCursor(Qt::CrossCursor);
    qDebug()<<"drawLine";
}

void MainWindow::drawRect()
{
    m_currentMode = Mode_Rect;
    viewer->setCursor(Qt::CrossCursor);
}

void MainWindow::drawEllipse()
{
    m_currentMode = Mode_Ellipse;
    viewer->setCursor(Qt::CrossCursor);
}

void MainWindow::drawPoint()
{
    m_currentMode = Mode_Point;
    viewer->setCursor(Qt::CrossCursor);
}



bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == viewer) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {

                QPointF scenePos = viewer->mapToScene(mouseEvent->pos());
                // 获取 QGraphicsPixmapItem
                QGraphicsPixmapItem *pixmapItem = viewer->pixmapItem();

                if (m_currentMode == Mode_Point) {
                    if (pixmapItem && pixmapItem->contains(scenePos) && !m_image.isNull()) {
                        QPointF imagePos = pixmapItem->mapFromScene(scenePos);
                        int x = qRound(imagePos.x());
                        int y = qRound(imagePos.y());

                        // 像素值读取逻辑
                        if (m_image.valid(x, y)) {
                            QColor color = m_image.pixelColor(x, y);
                            int grayValue = qGray(color.rgb());

                            AnnotationPointItem *point = new AnnotationPointItem(scenePos.x(), scenePos.y(), pixmapItem, m_image);
                            point->setMeasurement(imagePos, grayValue);
                            viewer->scene()->addItem(point);

                            m_currentMode = Mode_None;
                            viewer->setCursor(Qt::ArrowCursor);
                        }
                    }
                    return true; // 绘制模式下，消费事件
                }
            }
        }

        // 非绘图模式下（Mode_None）或其他鼠标事件，让 QGraphicsView 的默认行为（如拖动平移）生效
        if (m_currentMode == Mode_None || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
            return QMainWindow::eventFilter(watched, event);
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
