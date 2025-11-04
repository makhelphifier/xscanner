#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QTimer>
#include <QToolBar>
#include <QDebug>
#include <QFileDialog>
#include <QMenuBar>
#include "gui/states/genericdrawingstate.h"
#include "gui/widgets/logwidget.h"
#include "util/logger/qtwidgetappender.h"
#include "log4qt/logger.h"
#include <QDockWidget>
#include "gui/widgets/toprightinfowidget.h"
#include "util/logger/logger.h"
#include "gui/items/rectroi.h"

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
    infoLabel->adjustSize();
    infoLabel->move(10, viewer->height() - infoLabel->height() - 10);
    infoLabel->setVisible(true);

    // --- 添加右上角信息控件 ---
    infoWidget = new TopRightInfoWidget(viewer);
    infoWidget->setVisible(false);

    // --- 添加日志控件 ---
    QDockWidget *logDockWidget = new QDockWidget("日志", this);
    m_logWidget = new LogWidget(this);
    logDockWidget->setWidget(m_logWidget);
    addDockWidget(Qt::BottomDockWidgetArea, logDockWidget);

    // 将 Appender 的信号连接到 LogWidget 的槽 ---
    connect(QtWidgetAppender::instance(), &QtWidgetAppender::messageAppended,
            m_logWidget, QOverload<const QString &, int>::of(&LogWidget::appendLogMessage),
            Qt::QueuedConnection);
    // 将 LogWidget 的级别更改信号连接到 MainWindow 的槽 ---
    connect(m_logWidget, &LogWidget::logLevelChanged, this, &MainWindow::onLogLevelChanged);



    // --- 菜单和工具栏 ---
    QMenu *fileMenu = menuBar()->addMenu("文件");
    openAction = fileMenu->addAction("打开");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    QToolBar *toolBar = new QToolBar("测量工具", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);
    toolGroup = new QActionGroup(this);

    // 选择工具
    selectAction = new QAction(QIcon(":/Resources/img/arrow_tool.png"), "选择/拖动", this);
    selectAction->setCheckable(true);
    toolBar->addAction(selectAction);
    toolGroup->addAction(selectAction);
    connect(selectAction, &QAction::triggered, this, [this]() {
        viewer->setToolMode(ImageViewer::ModeSelect);
    });

    // 窗宽窗位工具
    wlAction = new QAction(QIcon(":/Resources/img/u27.png"), "窗宽窗位", this);
    wlAction->setCheckable(true);
    // toolBar->addAction(wlAction);
    toolGroup->addAction(wlAction);
    connect(wlAction, &QAction::triggered, [this]() {});

    // 直线工具
    lineAction = new QAction(QIcon(":/Resources/img/line_tool.png"), "直线", this);
    lineAction->setCheckable(true);
    toolBar->addAction(lineAction);
    toolGroup->addAction(lineAction);
    connect(lineAction, &QAction::triggered, [this]() {  viewer->setToolMode(ImageViewer::ModeDrawLine);});

    // 矩形工具
    rectAction = new QAction(QIcon(":/Resources/img/rect_tool.png"), "矩形", this);
    rectAction->setCheckable(true);
    toolBar->addAction(rectAction);
    toolGroup->addAction(rectAction);
    connect(rectAction, &QAction::triggered, [this]() {
        viewer->setToolMode(ImageViewer::ModeDrawRect);
    });

    // 椭圆工具
    ellipseAction = new QAction(QIcon(":/Resources/img/ellipse_tool.png"), "椭圆", this);
    ellipseAction->setCheckable(true);
    toolBar->addAction(ellipseAction);
    toolGroup->addAction(ellipseAction);
    connect(ellipseAction, &QAction::triggered, [this]() {
        viewer->setToolMode(ImageViewer::ModeDrawEllipse);
    });

    // 点测量工具
    pointAction = new QAction(QIcon(":/Resources/img/masure.png"), "点测量", this);
    pointAction->setCheckable(true);
    toolBar->addAction(pointAction);
    toolGroup->addAction(pointAction);
    connect(pointAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawPoint);});

    // 水平线工具
    hLineAction = new QAction(QIcon(":/Resources/img/u26.png"), "水平线", this);
    hLineAction->setCheckable(true);
    toolBar->addAction(hLineAction);
    toolGroup->addAction(hLineAction);
    connect(hLineAction, &QAction::triggered, [this]() {  viewer->setToolMode(ImageViewer::ModeDrawHLine);});

    // 垂直线工具
    vLineAction = new QAction(QIcon(":/Resources/img/u25.png"), "垂直线", this);
    vLineAction->setCheckable(true);
    toolBar->addAction(vLineAction);
    toolGroup->addAction(vLineAction);
    connect(vLineAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawVLine);});

    angledLineAction = new QAction(QIcon(":/Resources/img/u28.png"), "倾斜直线", this); // u28.png 是一条斜线
    angledLineAction->setCheckable(true);
    toolBar->addAction(angledLineAction);
    toolGroup->addAction(angledLineAction);
    connect(angledLineAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawAngledLine);});

    angleAction = new QAction(QIcon(":/Resources/img/masure.png"), "角度测量", this);
    angleAction->setCheckable(true);
    toolBar->addAction(angleAction);
    toolGroup->addAction(angleAction);
    connect(angleAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawAngle);});

    polylineAction = new QAction(QIcon(":/Resources/img/pen_tool.png"), "折线", this);
    polylineAction->setCheckable(true);
    toolBar->addAction(polylineAction);
    toolGroup->addAction(polylineAction);
    connect(polylineAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawPolyline);});

    freehandAction = new QAction(QIcon(":/Resources/img/color_tool.png"), "自由轨迹", this);
    freehandAction->setCheckable(true);
    toolBar->addAction(freehandAction);
    toolGroup->addAction(freehandAction);
    connect(freehandAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawFreehand);});

    textAction = new QAction(QIcon(":/Resources/img/text_tool.png"), "文本", this);
    textAction->setCheckable(true);
    toolBar->addAction(textAction);
    toolGroup->addAction(textAction);
    connect(textAction, &QAction::triggered, [this]() { viewer->setToolMode(ImageViewer::ModeDrawText);});

    // 添加分隔符
    toolBar->addSeparator();

    // 清除所有图元工具
    clearAction = new QAction(QIcon(":/Resources/img/empty_trash.png"), "", this);
    // toolBar->addAction(clearAction);
    // connect(clearAction, &QAction::triggered, viewer, &ImageViewer::clearAllAnnotations);

    // 在连接信号部分添加
    connect(ellipseAction, &QAction::triggered, [this]() {  });

    // 默认选择模式（确保 pointAction 未选中）
    selectAction->setChecked(true);

    // --- 连接信号和槽 ---
    // 缩放相关
    connect(viewer, &ImageViewer::scaleChanged, this, &MainWindow::updateScale);
    // connect(viewer, &ImageViewer::scaleChanged, viewer, &ImageViewer::onScaleChanged);
    connect(infoWidget, QOverload<double>::of(&TopRightInfoWidget::scaleEdited), this, &MainWindow::onScaleFromWidget);

    // 像素信息
    connect(viewer, &ImageViewer::pixelInfoChanged, this, &MainWindow::onPixelInfoChanged);

    // 窗宽窗位相关
    connect(infoWidget, &TopRightInfoWidget::windowChanged, viewer, &ImageViewer::onWindowChanged);
    connect(infoWidget, &TopRightInfoWidget::levelChanged, viewer, &ImageViewer::onLevelChanged);
    connect(infoWidget, &TopRightInfoWidget::autoWindowingToggled, viewer, &ImageViewer::setAutoWindowing);

    connect(viewer, &ImageViewer::windowLevelChanged, this, &MainWindow::onWindowLevelChanged);
    connect(viewer, &ImageViewer::autoWindowingToggled, this, &MainWindow::onAutoWindowingToggled);

    // 默认选择模式
    selectAction->setChecked(true);

    QString exeDir = QCoreApplication::applicationDirPath();
    QString filePath = QDir(exeDir).filePath("Resources/img/000006.raw");
    qDebug() << "Attempting to load from filesystem:" << filePath;
    // 默认加载图像（委托给 viewer）
    QString defaultPath = ":/Resources/img/000006.raw";
    viewer->loadImage(filePath);


    // 延迟更新 UI（等待图像加载）
    QTimer::singleShot(100, this, [this]() {
        if (!viewer->pixmapItem() || viewer->pixmapItem()->pixmap().isNull()) return;

        // 更新尺寸标签
        int width = viewer->pixmapItem()->pixmap().width();
        int height = viewer->pixmapItem()->pixmap().height();
        sizeLabel->setText(QString("size: %1x%2").arg(width).arg(height));
        sizeLabel->adjustSize();
        sizeLabel->setVisible(true);

        // 更新 infoWidget 可见性和范围（从 viewer 查询位深）
        infoWidget->setVisible(true);
        int bitDepth = viewer->bitDepth();
        int maxVal = (bitDepth == 16) ? 65535 : 255;
        infoWidget->setWindowRange(1, maxVal + 1);
        infoWidget->setLevelRange(0, maxVal);

        // 初始化 UI 到当前值（从 viewer 查询）
        infoWidget->setWindowValue(viewer->currentWindowWidth());
        infoWidget->setLevelValue(viewer->currentWindowLevel());
        infoWidget->setWindowLevelText(QString("window/level: %1/%2")
                                           .arg(viewer->currentWindowWidth())
                                           .arg(viewer->currentWindowLevel()));

        // 启用自动窗宽
        infoWidget->setAutoWindowingChecked(true);
        viewer->setAutoWindowing(true);  // 确保 viewer 同步
        // LogDebug("This is a debug message.");
        // LogWarn("This is a warning message.");
        // LogInfo("This is a info message.");
        // LogError_("This is an error message.");
        // LogFatal("This is a fatal message.");
    });

    // 手动调用 resizeEvent
    QTimer::singleShot(0, this, [this](){ resizeEvent(nullptr); });
}

MainWindow::~MainWindow() {}

void MainWindow::openImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "打开图片", "", "Raw Binary (*.raw *.bin);;Images (*.png *.jpg *.bmp)");

    if (filePath.isEmpty()) return;

    qDebug() << "Selected file path:" << filePath;

    // 委托加载给 viewer
    viewer->loadImage(filePath);

    // 延迟更新 UI（等待加载完成）
    QTimer::singleShot(100, this, [this, filePath]() {
        if (!viewer->pixmapItem() || viewer->pixmapItem()->pixmap().isNull()) {
            qDebug() << "Failed to load image:" << filePath;
            sizeLabel->setVisible(false);
            infoWidget->setVisible(false);
            infoLabel->setText("X: N/A, Y: N/A, value: N/A");
            return;
        }

        // 更新尺寸
        int width = viewer->pixmapItem()->pixmap().width();
        int height = viewer->pixmapItem()->pixmap().height();
        sizeLabel->setText(QString("size: %1x%2").arg(width).arg(height));
        sizeLabel->adjustSize();
        sizeLabel->setVisible(true);

        // 更新 infoWidget
        infoWidget->setVisible(true);
        int bitDepth = viewer->bitDepth();
        int maxVal = (bitDepth == 16) ? 65535 : 255;
        infoWidget->setWindowRange(1, maxVal + 1);
        infoWidget->setLevelRange(0, maxVal);

        // 从 viewer 获取当前值（加载后默认为全范围）
        infoWidget->setWindowValue(viewer->currentWindowWidth());
        infoWidget->setLevelValue(viewer->currentWindowLevel());
        infoWidget->setWindowLevelText(QString("window/level: %1/%2")
                                           .arg(viewer->currentWindowWidth())
                                           .arg(viewer->currentWindowLevel()));

        // 默认启用自动窗宽
        infoWidget->setAutoWindowingChecked(true);
        viewer->setAutoWindowing(true);
    });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (infoLabel) {
        infoLabel->move(10, viewer->height() - infoLabel->height() - 10);
    }
    if (infoWidget) {
        infoWidget->move(viewer->width() - infoWidget->width() - 10, 10);
    }
}


void MainWindow::onPixelInfoChanged(int x, int y, int value)
{
    QString infoText;
    if (value != -1) {
        infoText = QString("X: %1, Y: %2, value: %3").arg(x).arg(y).arg(value);
    } else {
        infoText = QString("X: %1, Y: %2, value: N/A").arg(x).arg(y);
    }
    infoLabel->setText(infoText);
    infoLabel->adjustSize();
}

void MainWindow::onWindowLevelChanged(int width, int level)
{
    infoWidget->setWindowValue(width);
    infoWidget->setLevelValue(level);

    infoWidget->setWindowLevelText(QString("window/level: %1/%2").arg(width).arg(level));
}

void MainWindow::onAutoWindowingToggled(bool enabled)
{
    // 更新复选框
    if (enabled) {
        infoWidget->checkAutoWindowing();
    } else {
        infoWidget->uncheckAutoWindowing();
    }
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
        viewer->setScale(static_cast<qreal>(scale));
    }
}

void MainWindow::onLogLevelChanged(Log4Qt::Level level)
{
    Log4Qt::Logger::rootLogger()->setLevel(level);
    // LogInfo(QString("Log level changed to %1").arg(level.toString()));
}
