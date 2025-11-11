// [修复] 修正 include 路径
#include "views/mainwindow.h"
#include "logger.h"
#include "qtwidgetappender.h"
// (c3100amotionwidget 的路径也已修正，尽管它当前被注释掉了)

#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"
#include "log4qt/propertyconfigurator.h"
#include "motion/c3100amotionwidget.h"

#include <QApplication>
#include <QDir>
#include <QThread>
#include <QPlainTextEdit>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug()<<"ssss";
    // 1. 设置主线程名称
    QThread::currentThread()->setObjectName("MainThread");

    // 2.直接在主线程同步初始化日志系统
    Log4Qt::PropertyConfigurator::configure("log4qt.properties");
    qDebug()<<"ssss";

    // 3. 配置 QtWidgetAppender (MainWindow 会自动连接到它)
    Log4Qt::PatternLayout *pLayout = new Log4Qt::PatternLayout(&a); // 传递父对象
    pLayout->setConversionPattern(QStringLiteral("[%d{yyyy-MM-dd HH:mm:ss.zzz}][%-20t][%l][%-5p] --> %m %n"));
    pLayout->activateOptions();
    QtWidgetAppender::instance()->setLayout(pLayout);
    Log4Qt::Logger::rootLogger()->addAppender(QtWidgetAppender::instance());
    qDebug()<<"ssss";

    Logger::instance();

    MainWindow w;
    w.show();
    qDebug()<<"ssss";

    C3100AMotionWidget *my3100UI = new C3100AMotionWidget();
    my3100UI->setWindowTitle("3100A 扫描设备控制");
    my3100UI->show();

    return a.exec();
}
