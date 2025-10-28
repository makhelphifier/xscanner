#include "gui/views/mainwindow.h"
#include "util/logger/logger.h"
#include "gui/widgets/logwidget.h"
#include "util/logger/qtwidgetappender.h"
#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"

#include <QApplication>
#include <QDir>
#include <QThread>
#include <QPlainTextEdit>

#include "device/gongy.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QThread::currentThread()->setObjectName("MainThread");

    Logger::init("log4qt.properties");

    LogInfo("====================================");
    LogInfo(QString("Application starting... Logger initialized from properties."));
    LogDebug("This is a DEBUG message.");
    LogInfo("This is an INFO message.");
    LogWarn("This is a WARN message.");
    LogError("This is an ERROR message.");
    LogFatal("This is a FATAL message.");

    MainWindow w;
    w.show();

    GongYTabWidget *colleagueWindow = new GongYTabWidget();
    colleagueWindow->setWindowTitle("扫描设备控制");
    colleagueWindow->show();


    // 创建 LogWidget 实例
    LogWidget *logWindow = new LogWidget();
    logWindow->setWindowTitle("实时日志窗口");
    logWindow->show();

    // 创建 QtWidgetAppender 实例
    QtWidgetAppender *widgetAppender = new QtWidgetAppender(&a); // 'a' 作为父对象

    // 创建一个布局 (格式)
    Log4Qt::PatternLayout *layout = new Log4Qt::PatternLayout(Log4Qt::PatternLayout::TTCC_CONVERSION_PATTERN, widgetAppender);
    layout->activateOptions();
    widgetAppender->setLayout(layout);
    widgetAppender->activateOptions();

    // 关键：将 Appender (发射塔) 添加到 root logger
    Log4Qt::Logger::rootLogger()->addAppender(widgetAppender);




    LogInfo("Windows shown. Application entering event loop.");
    LogInfo("LogWidget should now be receiving messages.");

    return a.exec();
}

