#ifndef QTWIDGETAPPENDER_H
#define QTWIDGETAPPENDER_H

#include "log4qt/appenderskeleton.h"
#include <QObject>
#include <QString>

/**
 * @class QtWidgetAppender
 * @brief 一个自定义的 Log4Qt Appender，它通过 Qt 信号发出日志消息。
 *
 * 这个 Appender 会接收 log4qt 的日志事件，
 * 将它们格式化为字符串，然后通过 'messageAppended(QString)' 信号发射出去。
 * 任何 QObject (例如 LogWidget) 都可以连接到这个信号来接收日志。
 */
class QtWidgetAppender : public Log4Qt::AppenderSkeleton
{
    Q_OBJECT

public:
    explicit QtWidgetAppender(QObject *parent = nullptr);
    virtual ~QtWidgetAppender();

    // 这个 appender 不需要立即激活，设为 false
    virtual bool requiresLayout() const override;

protected:
    /**
     * @brief Log4Qt 会调用这个虚函数来处理日志事件。
     * 我们在这里将其格式化并发射信号。
     */
    virtual void append(const Log4Qt::LoggingEvent &rEvent) override;

signals:
    /**
     * @brief 当有新消息需要追加时，发射此信号。
     * @param message 已经由 Layout 格式化好的日志字符串。
     */
    void messageAppended(const QString &message);

private:
    Q_DISABLE_COPY(QtWidgetAppender)
};

#endif // QTWIDGETAPPENDER_H
