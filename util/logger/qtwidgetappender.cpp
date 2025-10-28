#include "qtwidgetappender.h"
#include "log4qt/loggingevent.h"
#include "log4qt/layout.h"

QtWidgetAppender::QtWidgetAppender(QObject *parent)
    : Log4Qt::AppenderSkeleton(parent)
{
    // 构造函数
}

QtWidgetAppender::~QtWidgetAppender()
{
    // 析构函数
    // 在父对象 QObject 销毁时，这个 Appender 会自动从 root logger 中移除
    close();
}

bool QtWidgetAppender::requiresLayout() const
{
    // 我们需要一个 Layout 来将日志事件转换为字符串
    return true;
}

void QtWidgetAppender::append(const Log4Qt::LoggingEvent &rEvent)
{
    // 检查日志级别是否通过
    if (rEvent.level() < threshold()) {
        return;
    }

    // 使用 layout() (从 AppenderSkeleton 继承) 来格式化日志事件
    const QString message = layout()->format(rEvent);

    // 发射信号，将格式化后的消息传递出去
    // 确保这个信号是在主线程（GUI线程）上处理的
    // LogWidget 必须在主线程创建
    emit messageAppended(message);
}
