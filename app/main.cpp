#include "gui/views/mainwindow.h"

#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"

#include "util/logger/logger.h"
#include "util/logger/qtwidgetappender.h"

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
    Logger::init(configPath);

    Log4Qt::PatternLayout *pLayout = new Log4Qt::PatternLayout(&a);
    pLayout->setConversionPattern(QStringLiteral("[%d{yyyy-MM-dd HH:mm:ss.zzz}][%-20t][%l][%-5p] --> %m %n"));
    pLayout->activateOptions();
    QtWidgetAppender::instance()->setLayout(pLayout);

    Log4Qt::Logger::rootLogger()->addAppender(QtWidgetAppender::instance());

    LogInfo("====================================");
    LogInfo("Application starting...");
    LogInfo(QString("Config file loaded from: %1").arg(configPath));


    MainWindow w;
    w.show();

    GongYTabWidget *colleagueWindow = new GongYTabWidget();
    colleagueWindow->setWindowTitle("扫描设备控制"); // (可选)给个标题
    colleagueWindow->show();

    return a.exec();
}
