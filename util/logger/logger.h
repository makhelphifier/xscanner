#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

// 前向声明
namespace Log4Qt
{
class Logger;
}

class Logger : public QObject
{
    Q_OBJECT
private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger() override;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    static Logger* instance();
    static void init(const QString& confPath);

    // 关键改动：提供一个通用的 log 方法，接收所有信息
    void log(const char* file, int line, const char* function, const QString& level, const QString& message);

private:
    Log4Qt::Logger *m_rootLogger;
};

// 关键改动：升级宏，自动传递 __FILE__, __LINE__ 等信息
// Q_FUNC_INFO 是 Qt 提供的宏，能更好地展示函数签名
#define LogDebug(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "DEBUG", message)
#define LogInfo(message)  Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "INFO", message)
#define LogWarn(message)  Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "WARN", message)
#define LogError(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "ERROR", message)
#define LogFatal(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "FATAL", message)

#endif // LOGGER_H
