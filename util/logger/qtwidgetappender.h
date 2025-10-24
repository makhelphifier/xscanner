#ifndef QTWIDGETAPPENDER_H
#define QTWIDGETAPPENDER_H

#include "log4qt/appenderskeleton.h"
#include <QObject>

// 前向声明
namespace Log4Qt {
class LoggingEvent;
}

class QtWidgetAppender : public QObject, public Log4Qt::AppenderSkeleton
{
    Q_OBJECT

private:
    explicit QtWidgetAppender(QObject *parent = nullptr);
    ~QtWidgetAppender() override;
    QtWidgetAppender(const QtWidgetAppender&) = delete;
    QtWidgetAppender& operator=(const QtWidgetAppender&) = delete;

public:
    static QtWidgetAppender* instance();

    // AppenderSkeleton 接口的实现
    bool requiresLayout() const override { return true; }

protected:
    void append(const Log4Qt::LoggingEvent &rEvent) override;

Q_SIGNALS:
    // 定义信号，将日志事件的格式化文本和级别发送出去
    void messageAppended(const QString &formattedMessage, int level);
};

#endif // QTWIDGETAPPENDER_H
