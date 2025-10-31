#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#if defined(_MSC_VER)  // MSVC 编译器
#define __FUNCTION_NAME__ (strrchr(__FUNCTION__, ':') ? strrchr(__FUNCTION__, ':') + 1 : __FUNCTION__)
#else  // GCC/Clang 等，使用 __func__（已经是纯函数名）
#define __FUNCTION_NAME__ __func__
#endif

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

    void log(const char* file, int line, const char* function, const QString& level, const QString& message);

private:
    Log4Qt::Logger *m_rootLogger;
};

#define LogDebug(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "DEBUG", message)
#define LogInfo(message)  Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "INFO", message)
#define log_(message)  Logger::instance()->log(__FILE__, __LINE__, __FUNCTION_NAME__, "INFO", message)
#define LogWarn(message)  Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "WARN", message)
#define LogError_(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "ERROR", message) //解决宏定义冲突，
#define LogFatal(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "FATAL", message)

#endif // LOGGER_H
