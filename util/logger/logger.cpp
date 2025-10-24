// --- 首先包含 Log4Qt 的头文件 ---
#include "log4qt/logger.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"
#include "log4qt/loggingevent.h" // 包含 LoggingEvent 和 MessageContext
#include "log4qt/level.h"       // Level

// --- 然后再包含我们自己的头文件 ---
#include "logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QHash>  // 新增：用于 QHash<QString, QString> (properties/MDC)

static Logger* g_logger_instance = nullptr;

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    m_rootLogger = Log4Qt::LogManager::rootLogger();
}

Logger::~Logger() {}

Logger* Logger::instance()
{
    if (!g_logger_instance) {
        g_logger_instance = new Logger(QCoreApplication::instance());
    }
    return g_logger_instance;
}

void Logger::init(const QString& confPath)
{
    Log4Qt::PropertyConfigurator::configure(confPath);
    LogInfo("Logger system initialized successfully."); // 现在这里的 LogInfo 也会有正确的位置信息了
}

// 3. --- 全新的 log 方法实现（最终修正版） ---
void Logger::log(const char* file, int line, const char* function, const QString& level, const QString& message)
{
    if (!m_rootLogger) return;

    // 将字符串级别的 "DEBUG", "INFO" 等转换为 Log4Qt 的 Level 对象
    Log4Qt::Level qtLevel = Log4Qt::Level::fromString(level);

    // 获取当前线程名：优先使用 objectName，如果为空则 fallback 到线程 ID 字符串
    QString threadName;
    QThread* currentThread = QThread::currentThread();
    if (!currentThread->objectName().isEmpty()) {
        threadName = currentThread->objectName();  // 如 "MainThread"
    } else {
        // Fallback: 使用线程 ID（实例方法 currentThread()->threadId()，转换为十六进制字符串）
        Qt::HANDLE threadId = currentThread->currentThreadId();  // Qt::HANDLE 是 void*
        threadName = QString("Thread-%1").arg(reinterpret_cast<quintptr>(threadId), 0, 16);
    }

    // 时间戳：转换为 qint64（毫秒自 1970-01-01）
    qint64 timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // 位置上下文：使用 MessageContext（直接支持 const char* file 和 function）
    Log4Qt::MessageContext context(file, line, function);

    // 创建一个包含所有上下文信息的 LoggingEvent 对象（使用完整构造函数）
    Log4Qt::LoggingEvent event(m_rootLogger,                // const Logger*
                               qtLevel,
                               message,
                               QString(),                    // ndc: 空（嵌套诊断上下文）
                               QHash<QString, QString>(),    // properties/MDC: 空哈希
                               threadName,
                               timeStamp,
                               context,
                               QString());                   // categoryName: 空（或 m_rootLogger->loggerName()）

    // 将这个完整的事件分发给所有 Appender
    m_rootLogger->callAppenders(event);
}
