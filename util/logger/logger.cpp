#include "log4qt/logger.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"
#include "log4qt/loggingevent.h"
#include "log4qt/level.h"

#include "logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QHash>

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
    LogInfo("Logger system initialized successfully.");
}

void Logger::log(const char* file, int line, const char* function, const QString& level, const QString& message)
{
    if (!m_rootLogger) return;

    Log4Qt::Level qtLevel = Log4Qt::Level::fromString(level);

    QString threadName;
    QThread* currentThread = QThread::currentThread();
    if (!currentThread->objectName().isEmpty()) {
        threadName = currentThread->objectName();
    } else {
        Qt::HANDLE threadId = currentThread->currentThreadId();
        threadName = QString("Thread-%1").arg(reinterpret_cast<quintptr>(threadId), 0, 16);
    }

    qint64 timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    Log4Qt::MessageContext context(file, line, "function");

    Log4Qt::LoggingEvent event(m_rootLogger,
                               qtLevel,
                               message,
                               QString(),
                               QHash<QString, QString>(),
                               threadName,
                               timeStamp,
                               context,
                               QString());

    m_rootLogger->callAppenders(event);
}
