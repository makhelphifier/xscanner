// util/logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include "log4qt/logger.h"

/*
 * @brief 方便调用的日志宏
 * * 使用方法:
 * LogDebug("This is a debug message.");
 * LogInfo(QString("Value is %1").arg(123));
 */
#define LogDebug(message) Logger::instance()->debug(message)
#define LogInfo(message)  Logger::instance()->info(message)
#define LogWarn(message)  Logger::instance()->warn(message)
#define LogError(message) Logger::instance()->error(message)
#define LogFatal(message) Logger::instance()->fatal(message)

/**
 * @class Logger
 * @brief 日志系统单例封装类
 *
 * 封装 log4qt，提供全局访问点和初始化。
 * 必须在 main() 函数中调用 Logger::init() 来初始化。
 */
class Logger : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 获取 Logger 的单例实例
     */
    static Logger* instance();

    /**
     * @brief 初始化日志系统
     * @param configFilePath 配置文件的路径（通常是相对于可执行文件的路径）
     * * @note 必须在程序启动时（例如 main.cpp 中）调用一次。
     */
    static void init(const QString& configFilePath);

    // 各种日志级别的接口
    void debug(const QString& message);
    void info(const QString& message);
    void warn(const QString& message);
    void error(const QString& message);
    void fatal(const QString& message);

private:
    // 私有构造函数，防止外部实例化
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger* m_instance; // 单例实例
    Log4Qt::Logger* m_rootLogger; // log4qt 的 root logger
};

#endif // LOGGER_H
