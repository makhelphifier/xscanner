#include <QApplication>
#include <QDebug>

// 包含您想测试的 xscanner_gui 库中的头文件
#include "widgets/logwidget.h"
#include "logger.h" // 也可以测试 logger
#include "qtwidgetappender.h"
#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"
#include "log4qt/propertyconfigurator.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // --- 简单的日志设置 (用于测试 LogWidget) ---
    Log4Qt::PropertyConfigurator::configure("log4qt.properties");
    Log4Qt::PatternLayout *pLayout = new Log4Qt::PatternLayout(&a);
    pLayout->setConversionPattern(QStringLiteral("[%d{HH:mm:ss.zzz}][%-5p] %m %n"));
    pLayout->activateOptions();
    QtWidgetAppender::instance()->setLayout(pLayout);
    Log4Qt::Logger::rootLogger()->addAppender(QtWidgetAppender::instance());
    // --- 日志设置结束 ---

    // 创建 LogWidget 实例
    LogWidget w;
    w.setWindowTitle("GUI Demo Test - LogWidget");
    w.resize(600, 300);
    w.show();

    // 发送一些日志
    qDebug() << "这是一个 DEBUG 消息 (qDebug)";
    LogInfo("这是一个 INFO 消息 (LogInfo)");
    LogWarn("这是一个 WARN 消息 (LogWarn)");

    return a.exec();
}
