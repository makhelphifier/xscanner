#include "gui/views/mainwindow.h"

// --- 关键修正：首先包含 Log4Qt 的完整定义 ---
#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"

// --- 然后再包含我们自己的、依赖 Log4Qt 的工具类 ---
#include "util/logger/logger.h"
#include "util/logger/qtwidgetappender.h"

#include <QApplication>
#include <QDir>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QThread::currentThread()->setObjectName("MainThread");
    // 获取可执行文件所在目录
    QString configPath = a.applicationDirPath();
    // 拼接配置文件的完整路径
    configPath.append("/log4qt.properties");
    // 初始化日志系统
    Logger::init(configPath);

    // --- 为我们的 WidgetAppender 配置布局 ---
    // 1. 创建一个 PatternLayout
    Log4Qt::PatternLayout *pLayout = new Log4Qt::PatternLayout(&a); // 传递父对象
    // 2. 设置与 log4qt.properties 中一致的格式
    pLayout->setConversionPattern(QStringLiteral("[%d{yyyy-MM-dd HH:mm:ss.zzz}][%-20t][%l][%-5p] --> %m %n"));
    pLayout->activateOptions();
    // 3. 将此布局设置给我们的自定义 Appender
    QtWidgetAppender::instance()->setLayout(pLayout);


    // 将我们的自定义 WidgetAppender 添加到根 Logger
    Log4Qt::Logger::rootLogger()->addAppender(QtWidgetAppender::instance());

    LogInfo("====================================");
    LogInfo("Application starting...");
    LogInfo(QString("Config file loaded from: %1").arg(configPath));


    MainWindow w;
    w.show();
    return a.exec();
}
