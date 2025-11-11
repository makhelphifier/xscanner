#include "logger.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"
#include "log4qt/loggingevent.h"
#include "log4qt/level.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QThread>


LoggerWorker::LoggerWorker(QObject *parent) : QObject(parent)
{
    m_rootLogger = Log4Qt::LogManager::rootLogger();
}


void LoggerWorker::processLogRequest(const QString &file, int line, const QString &function, const QString &level, const QString &message)
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

    QByteArray fileBytes = file.toUtf8();
    QByteArray funcBytes = function.toUtf8();
    Log4Qt::MessageContext context(fileBytes.constData(), line, funcBytes.constData());

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



Logger* Logger::g_logger_instance = nullptr;

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    m_worker = new LoggerWorker;
    m_worker->moveToThread(&m_workerThread);

    connect(this, &Logger::logRequested, m_worker, &LoggerWorker::processLogRequest);
    // connect(this, &Logger::initRequested, m_worker, &LoggerWorker::initLogger);

    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread.setObjectName("LoggerThread");
    m_workerThread.start();
}

Logger::~Logger()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

Logger* Logger::instance()
{
    if (!g_logger_instance) {
        g_logger_instance = new Logger(QCoreApplication::instance());
    }
    return g_logger_instance;
}


void Logger::log(const char* file, int line, const char* function, const QString& level, const QString& message)
{
    emit logRequested(QString(file), line, QString(function), level, message);
}
