#include "qtwidgetappender.h"
#include "log4qt/loggingevent.h"
#include "log4qt/layout.h"
#include "log4qt/level.h"

static QtWidgetAppender* g_widget_appender_instance = nullptr;

QtWidgetAppender::QtWidgetAppender(QObject *parent) :Log4Qt::AppenderSkeleton(parent)
{
}

QtWidgetAppender::~QtWidgetAppender()
{
    close();
}

QtWidgetAppender* QtWidgetAppender::instance()
{
    if (!g_widget_appender_instance) {
        g_widget_appender_instance = new QtWidgetAppender();
    }
    return g_widget_appender_instance;
}

void QtWidgetAppender::append(const Log4Qt::LoggingEvent &rEvent)
{
    // 使用 Appender 自己的布局格式化消息
    QString message = layout()->format(rEvent);
    // 发射信号，传递格式化后的消息和日志级别
    emit messageAppended(message, rEvent.level().toInt());
}
