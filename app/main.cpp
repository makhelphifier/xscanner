#include "gui/views/mainwindow.h"
#include "util/logger/logger.h"
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

    LogInfo("====================================");
    LogInfo("Application starting...");
    LogInfo(QString("Config file loaded from: %1").arg(configPath));


    MainWindow w;
    w.show();
    return a.exec();
}
