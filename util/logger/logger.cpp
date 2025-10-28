// util/logger.cpp
#include "logger.h"
#include "log4qt/propertyconfigurator.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug> // 用于在日志系统初始化失败时输出

// 初始化静态实例
Logger* Logger::m_instance = nullptr;

Logger* Logger::instance()
{
    // 注意：这里没有使用锁，假定 init() 会在主线程最开始被调用
    // 如果实例还未创建，则创建它
    if (m_instance == nullptr) {
        // 使用 QCoreApplication::instance() 作为父对象，以便在应用退出时自动清理
        m_instance = new Logger(QCoreApplication::instance());
    }
    return m_instance;
}

void Logger::init(const QString& configFilePath)
{
    // 确保实例已创建
    (void)instance();

    // 配置 log4qt
    // 假定配置文件位于可执行文件旁边
    QString fullPath = configFilePath;
    if (QCoreApplication::instance()) {
        fullPath = QCoreApplication::applicationDirPath() + QDir::separator() + configFilePath;
    }

    if (Log4Qt::PropertyConfigurator::configure(fullPath)) {
        // 使用刚配置好的日志系统记录第一条信息
        instance()->info(QString("Logger initialized successfully from: %1").arg(fullPath));
    } else {
        // 如果配置失败，log4qt 会自动在 stderr 打印错误
        // 我们也使用 Qt 的 qWarning 再次强调
        qWarning() << "Failed to initialize logger from: " << fullPath;
    }
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    // 获取 root logger
    m_rootLogger = Log4Qt::Logger::rootLogger();
}

Logger::~Logger()
{
    // log4qt 会在 QCoreApplication 销毁时自动关闭
    if (m_rootLogger) {
        m_rootLogger->info("Logger shutting down.");
    }
}

void Logger::debug(const QString& message)
{
    m_rootLogger->debug(message);
}

void Logger::info(const QString& message)
{
    m_rootLogger->info(message);
}

void Logger::warn(const QString& message)
{
    m_rootLogger->warn(message);
}

void Logger::error(const QString& message)
{
    m_rootLogger->error(message);
}

void Logger::fatal(const QString& message)
{
    m_rootLogger->fatal(message);
}
