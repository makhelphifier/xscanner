#include "gui/views/mainwindow.h"
#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"
#include "log4qt/consoleappender.h"      // ConsoleAppender 用于控制台输出
#include "log4qt/basicconfigurator.h"     // 新增：用于默认初始化配置
// #include "log4qt/propertyconfigurator.h"  // 如果用 properties 文件，可取消注释

// #include "util/logger/logger.h"
// #include "util/logger/qtwidgetappender.h"

#include <QApplication>
#include <QDir>
#include <QThread>

#include "device/gongy.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QThread::currentThread()->setObjectName("MainThread");
    QString configPath = a.applicationDirPath();
    configPath.append("/log4qt.properties");
    // Logger::init(configPath);  // 你的自定义 init，暂注释

    // 配置 Log4Qt（使用 BasicConfigurator 初始化默认控制台配置）
    Log4Qt::BasicConfigurator::configure();  // 这会设置默认 ConsoleAppender 和简单 Layout

    // 自定义 Layout（覆盖默认格式）
    Log4Qt::PatternLayout *pLayout = new Log4Qt::PatternLayout();
    pLayout->setConversionPattern(QStringLiteral("[%d{yyyy-MM-dd HH:mm:ss.zzz}][%-20t][%l][%-5p] --> %m %n"));
    pLayout->activateOptions();

    // 创建自定义 ConsoleAppender（替换默认的）
    Log4Qt::ConsoleAppender *consoleAppender = new Log4Qt::ConsoleAppender(pLayout);
    consoleAppender->activateOptions();

    // 获取根 Logger，移除默认 Appender（可选，避免重复输出），添加自定义 Appender
    Log4Qt::Logger *rootLogger = Log4Qt::Logger::rootLogger();
    rootLogger->removeAllAppenders();  // 移除 BasicConfigurator 添加的默认 Appender
    rootLogger->addAppender(consoleAppender);

    // 设置日志级别：INFO 及以上输出（DEBUG 等需改为 Log4Qt::Level::DEBUG()）
    rootLogger->setLevel(Log4Qt::Level::INFO_INT);

    // 测试打印日志
    rootLogger->info("====================================");
    rootLogger->info("Application starting...");
    rootLogger->info(QString("Config file path: %1").arg(configPath));
    rootLogger->debug("This is a DEBUG message (won't show if level is INFO)");  // 测试 DEBUG，不会输出
    rootLogger->warn("This is a WARN message");  // 会输出

    // 如果想用 properties 文件加载（可选，替换上面的手动配置）
    // Log4Qt::PropertyConfigurator::configure(configPath);

    MainWindow w;
    w.show();

    GongYTabWidget *colleagueWindow = new GongYTabWidget();
    colleagueWindow->setWindowTitle("扫描设备控制");
    colleagueWindow->show();

    return a.exec();
}
