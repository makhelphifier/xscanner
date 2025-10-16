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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    scene = new QGraphicsScene(this);

    QString imagePath = QCoreApplication::applicationDirPath() + "/image/aaa.jpg";

    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        qWarning() << "无法加载图片:" << imagePath;
    } else {
        m_image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    }

    pixmapItem = new QGraphicsPixmapItem(pixmap);
    scene->addItem(pixmapItem);

    view = new QGraphicsView(scene, this);
    QTimer::singleShot(0, this, [=]() {
        view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    });
    setCentralWidget(view);

    view->installEventFilter(this);



    QToolBar *toolBar = new QToolBar("测量工具", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);

    pointAction = new QAction("点", this);
    toolBar->addAction(pointAction);
    connect(pointAction, &QAction::triggered, this, &MainWindow::drawPoint);

    lineAction = new QAction("直线", this);

    toolBar->addAction(lineAction);

    connect(lineAction, &QAction::triggered, this, &MainWindow::drawLine);
    rectAction = new QAction("矩形", this);
    toolBar->addAction(rectAction);
    connect(rectAction, &QAction::triggered, this, &MainWindow::drawRect);

    ellipseAction = new QAction("椭圆", this);
    toolBar->addAction(ellipseAction);
    connect(ellipseAction, &QAction::triggered, this, &MainWindow::drawEllipse);


}


MainWindow::~MainWindow() {}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (view && scene && !scene->items().isEmpty()) {
        view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    }
}
void MainWindow::drawLine()
{
    m_currentMode = Mode_Line;
    view->setCursor(Qt::CrossCursor);
}

void MainWindow::drawRect()
{
    m_currentMode = Mode_Rect;
    view->setCursor(Qt::CrossCursor);
}

void MainWindow::drawEllipse()
{
    m_currentMode = Mode_Ellipse;
    view->setCursor(Qt::CrossCursor);
}

void MainWindow::drawPoint()
{
    m_currentMode = Mode_Point;
    view->setCursor(Qt::CrossCursor);
}



bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == view) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {

                QPointF scenePos = view->mapToScene(mouseEvent->pos());

                if (m_currentMode == Mode_Point) {
                    if (pixmapItem->contains(scenePos)) {
                        QPointF imagePos = pixmapItem->mapFromScene(scenePos);
                        int x = qRound(imagePos.x());
                        int y = qRound(imagePos.y());

                        QColor color = m_image.pixelColor(x, y);
                        int grayValue = color.red();

                        AnnotationPointItem *point = new AnnotationPointItem(scenePos.x(), scenePos.y(), pixmapItem, m_image);
                        point->setMeasurement(imagePos, grayValue);
                        scene->addItem(point);

                        m_currentMode = Mode_None;
                        view->setCursor(Qt::ArrowCursor);
                    }
                }

                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
