#include "gui/views/mainwindow.h"
#include "util/logger/logger.h"
#include "gui/widgets/logwidget.h"
#include "device/motionwidget.h"
#include "util/logger/qtwidgetappender.h"
#include "log4qt/logger.h"
#include "log4qt/patternlayout.h"

#include <QApplication>
#include <QDir>
#include <QThread>
#include <QPlainTextEdit>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QThread::currentThread()->setObjectName("MainThread");
    Logger::init("log4qt.properties");

    MainWindow w;
    w.show();

     GongYTabWidget *colleagueWindow = new GongYTabWidget();
    // colleagueWindow->setWindowTitle("扫描设备控制");
     colleagueWindow->show();

    Log4Qt::PatternLayout *pLayout = new Log4Qt::PatternLayout(&a); // 传递父对象
    pLayout->setConversionPattern(QStringLiteral("[%d{yyyy-MM-dd HH:mm:ss.zzz}][%-20t][%l][%-5p] --> %m %n"));
    pLayout->activateOptions();
    QtWidgetAppender::instance()->setLayout(pLayout);
    Log4Qt::Logger::rootLogger()->addAppender(QtWidgetAppender::instance());

    return a.exec();
}
