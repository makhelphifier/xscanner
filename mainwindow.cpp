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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    // 1. 初始化场景
    scene = new QGraphicsScene(this);

    // 2. 构造图片路径
    // 获取程序可执行文件所在的目录，并拼接上相对路径
    QString imagePath = QCoreApplication::applicationDirPath() + "/image/aaa.jpg";

    // 3. 加载图片并创建图片项
    QPixmap pixmap(imagePath);
    pixmapItem = new QGraphicsPixmapItem(pixmap);

    // 4. 将图片项添加到场景中
    scene->addItem(pixmapItem);

    // 5. 初始化视图并设置场景
    view = new QGraphicsView(scene, this);
    QTimer::singleShot(0, this, [=]() {
        view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    });
    // 6. 将视图设置为主窗口的中心控件
    setCentralWidget(view);

    // 1. 创建一个工具栏
    QToolBar *toolBar = addToolBar("主工具栏");

    // 2. 创建“直线”Action
    lineAction = new QAction("直线", this);

    // 3. 将Action添加到工具栏
    toolBar->addAction(lineAction);

    // 4. 连接Action的triggered()信号到我们定义的槽函数
    connect(lineAction, &QAction::triggered, this, &MainWindow::drawLine);
    // 创建“矩形”Action并添加到工具栏
    rectAction = new QAction("矩形", this);
    toolBar->addAction(rectAction);
    connect(rectAction, &QAction::triggered, this, &MainWindow::drawRect);

    // 创建“椭圆”Action并添加到工具栏
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
    // 之前这里是 qDebug() 输出，现在替换为以下代码

    // 1. 创建一个直线标注项实例
    // 为了演示，我们先在固定位置(50,50)到(200,150)创建
    AnnotationLineItem *line = new AnnotationLineItem(50, 50, 200, 150);

    // 2. 将这个直线项添加到场景中，它就会自动显示出来
    scene->addItem(line);
}


void MainWindow::drawRect()
{
    // 为了演示，在固定位置创建一个矩形标注项
    AnnotationRectItem *rect = new AnnotationRectItem(50, 50, 200, 150);
    scene->addItem(rect);
}

void MainWindow::drawEllipse()
{
    // 为了演示，在固定位置创建一个椭圆标注项
    AnnotationEllipseItem *ellipse = new AnnotationEllipseItem(50, 50, 200, 150);
    scene->addItem(ellipse);
}
