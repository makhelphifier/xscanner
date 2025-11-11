#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QThread>
#include "log4qt/logger.h"

class LoggerWorker : public QObject
{
    Q_OBJECT
private:
    Log4Qt::Logger *m_rootLogger;
public:
    explicit LoggerWorker(QObject *parent = nullptr);
    ~LoggerWorker() override = default;

public slots:
    // void ini/*t*/Logger(const QString& confPath);
    void processLogRequest(const QString &file, int line, const QString &function, const QString &level, const QString &message);
};


class Logger : public QObject
{
    Q_OBJECT
private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger() override;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LoggerWorker* m_worker;
    QThread m_workerThread;

public:
    static Logger* instance();
    // static void init(const QString& confPath);

    void log(const char* file, int line, const char* function, const QString& level, const QString& message);

private:
    static Logger* g_logger_instance;

signals:
    void logRequested(const QString &file, int line, const QString &function, const QString &level, const QString &message);
    // void initRequested(const QString& confPath);
};

#if defined(_MSC_VER)
#define __FUNCTION_NAME__ (strrchr(__FUNCTION__, ':') ? strrchr(__FUNCTION__, ':') + 1 : __FUNCTION__)
#else
#define __FUNCTION_NAME__ __func__
#endif

#define LogDebug(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "DEBUG", message)
#define LogInfo(message)  Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "INFO", message)
#define log_(message)  Logger::instance()->log(__FILE__, __LINE__, __FUNCTION_NAME__, "INFO", message)
#define LogWarn(message)  Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "WARN", message)
#define LogError_(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "ERROR", message)
#define LogFatal(message) Logger::instance()->log(__FILE__, __LINE__, Q_FUNC_INFO, "FATAL", message)

#endif // LOGGER_H
