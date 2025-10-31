#include "xraysource.h"
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrent>
// #include <serialcomunicator.h>
#include "serialcomunicator.h"

// Static member definitions
XraySource* XraySource::s_instance = nullptr;
QMutex XraySource::s_mutex;

XraySource* XraySource::getInstance()
{
    if(s_instance==nullptr)
    {
        QMutexLocker locker(&s_mutex);
        if (s_instance == nullptr) {
            s_instance = new XraySource();
        }
    }
    return s_instance;
}

void XraySource::destroyInstance()
{
    QMutexLocker locker(&s_mutex);
    if (s_instance != nullptr) {
        delete s_instance;
        s_instance = nullptr;
    }
}

XraySource::XraySource(QObject *parent)
    : QObject{parent}
{
    m_commandFuture = QtConcurrent::run([this](){


        m_serialPortComm = new SerialComunicator;
        connect(m_serialPortComm, &SerialComunicator::sig_didConnect, [this](int state, const QString& msg){
            qDebug()<<"&SerialPortViewModel::sig_didConnect: "<<state;
            emit sig_didConnect(state);
        });
        connect(m_serialPortComm, &SerialComunicator::sig_didDisconnect, [this](int state, const QString& msg){
            qDebug()<<"&SerialPortViewModel::sig_didDisconnect";
            emit sig_didDisconnect(state);
        });
        connect(m_serialPortComm, &SerialComunicator::sig_didLoadPortList, [=](const QList<StSerialPortInfo*>& itemList){
            QStringList nameList;
            for (int i = 0; i < itemList.count(); i++) {
                nameList << itemList.at(i)->description;
            }
            emit sig_didLoadPortsList(nameList);
        });
        connect(m_serialPortComm, &SerialComunicator::sig_didGetStatus, [this](StSerialPortInfo *item){
            emit sig_didGetXrayStatus(item->xrayInfo.warmingupState == 1 ? true : false,
                                      item->xrayInfo.interLock,
                                      item->xrayInfo.xRayOn,
                                      item->xrayInfo.currKv,
                                      item->xrayInfo.currUA);
        });

        m_serialPortComm->loadPorts();

        int timeCount = 0;
        while (m_commandLoop) {
            if (!m_commandQueue.isEmpty()) {

                StXrayCommand *cmd = m_commandQueue.dequeue();
                switch (cmd->type) {
                case XRAY_CMD_TYPE_CONNECT:
                {
                    if (m_serialPortComm->avaliblePorts().count() > cmd->value) {
                        m_serialPortComm->selPort(cmd->value);
                    }

                    break;
                }
                case XRAY_CMD_TYPE_DISCONNECT:
                {
                    m_serialPortComm->stopConnecting();
                    break;
                }
                case XRAY_CMD_TYPE_XRAY_ON:
                {
                    m_serialPortComm->xrayOn();
                    break;
                }
                case XRAY_CMD_TYPE_XRAY_OFF:
                {
                    m_serialPortComm->xrayOff();
                    break;
                }
                case XRAY_CMD_TYPE_XRAY_SET_KV:
                {
                    m_serialPortComm->setKv(cmd->value);
                    break;
                }
                case XRAY_CMD_TYPE_XRAY_SET_BEAM:
                {
                    m_serialPortComm->setBeam(cmd->value);
                    break;
                }
                case XRAY_CMD_TYPE_XRAY_GET_STATUS:
                {
                    m_gettingStatusInterval = cmd->value;
                    break;
                }
                default:
                {
                    break;
                }

                }

                delete cmd;


            }
            if (++timeCount > m_gettingStatusInterval) {
                timeCount = 0;

                m_serialPortComm->getStatus();
            }

            QThread::msleep(10);
        }

        m_serialPortComm->stopConnecting();
        m_serialPortComm->deleteLater();
    });

    // // 使用QObject的moveToThread代替QtConcurrent::run，更好更好的控制线程生命周期
    // m_workerThread = new QThread(this);
    // m_serialPortComm = new SerialComunicator();

    // // 将SerialComunicator移动到工作线程
    // m_serialPortComm->moveToThread(m_workerThread);

    // // 设置线程清理函数
    // connect(m_workerThread, &QThread::finished, m_serialPortComm, &QObject::deleteLater);
    // connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);

    // // 连接信号槽
    // connect(m_serialPortComm, &SerialComunicator::sig_didConnect, this, [this](int state, const QString& msg){
    //     qDebug() << "Connected state:" << state;
    //     emit sig_didConnect(state);
    // });

    // connect(m_serialPortComm, &SerialComunicator::sig_didDisconnect, this, [this](int state, const QString& msg){
    //     qDebug() << "Disconnected";
    //     emit sig_didDisconnect(state);
    // });

    // connect(m_serialPortComm, &SerialComunicator::sig_didLoadPortList, this, [this](const QList<StSerialPortInfo*>& itemList){
    //     QStringList nameList;
    //     for (const auto* item : itemList) {
    //         nameList << item->description;
    //     }
    //     emit sig_didLoadPortsList(nameList);
    // });

    // connect(m_serialPortComm, &SerialComunicator::sig_didGetStatus, this, [this](StSerialPortInfo *item){
    //     emit sig_didGetXrayStatus(
    //         item->xrayInfo.warmingupState == 1,
    //         item->xrayInfo.interLock,
    //         item->xrayInfo.xRayOn,
    //         item->xrayInfo.currKv,
    //         item->xrayInfo.currUA
    //         );
    // });

    // // 使用工作线程的处理函数
    // connect(this, &XraySource::sig_processCommands, this, &XraySource::onProcessCommands, Qt::QueuedConnection);

    // // 启动工作线程
    // m_workerThread->start();

    // // 加载端口列表
    // QMetaObject::invokeMethod(m_serialPortComm, "loadPorts", Qt::QueuedConnection);

    // // 启动命令处理循环
    // emit sig_processCommands();
}

XraySource::~XraySource()
{
    m_commandLoop = false;
    // m_commandFuture.cancel();
    // m_commandFuture.waitForFinished();

    if (!m_commandQueue.isEmpty()) {
        qDeleteAll(m_commandQueue);
        m_commandQueue.clear();
    }

    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void XraySource::onProcessCommands()
{
    static int timeCount = 0;

    if (!m_commandLoop) return;

    // 处理命令队列
    while (!m_commandQueue.isEmpty()) {
        StXrayCommand *cmd = m_commandQueue.dequeue();

        switch (cmd->type) {
        case XRAY_CMD_TYPE_CONNECT:
            if (m_serialPortComm->avaliblePorts().count() > cmd->value) {
                QMetaObject::invokeMethod(m_serialPortComm, "selPort", Qt::QueuedConnection,
                                          Q_ARG(int, cmd->value));
            }
            break;

        case XRAY_CMD_TYPE_DISCONNECT:
            QMetaObject::invokeMethod(m_serialPortComm, "stopConnecting", Qt::QueuedConnection);
            break;

        case XRAY_CMD_TYPE_XRAY_ON:
            QMetaObject::invokeMethod(m_serialPortComm, "xrayOn", Qt::QueuedConnection);
            break;

        case XRAY_CMD_TYPE_XRAY_OFF:
            QMetaObject::invokeMethod(m_serialPortComm, "xrayOff", Qt::QueuedConnection);
            break;

        case XRAY_CMD_TYPE_XRAY_SET_KV:
            QMetaObject::invokeMethod(m_serialPortComm, "setKv", Qt::QueuedConnection,
                                      Q_ARG(int, cmd->value));
            break;

        case XRAY_CMD_TYPE_XRAY_SET_BEAM:
            QMetaObject::invokeMethod(m_serialPortComm, "setBeam", Qt::QueuedConnection,
                                      Q_ARG(int, cmd->value));
            break;

        case XRAY_CMD_TYPE_XRAY_GET_STATUS:
            m_gettingStatusInterval = cmd->value;
            break;

        default:
            qWarning() << "Unknown command type:" << cmd->type;
            break;
        }

        delete cmd;
    }

    // 定期获取状态
    if (++timeCount > m_gettingStatusInterval) {
        timeCount = 0;
        QMetaObject::invokeMethod(m_serialPortComm, "getStatus", Qt::QueuedConnection);
    }


    // 继续处理循环
    if (m_commandLoop) {
        QTimer::singleShot(10, this, &XraySource::onProcessCommands);
    }
}

void XraySource::startToConnect(int index)
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_CONNECT;
    cmd->value = index;

    enqueueCommand(cmd);
}

void XraySource::stopToConnect()
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_DISCONNECT;


    enqueueCommand(cmd);
}

void XraySource::setXrayOn()
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_XRAY_ON;

    enqueueCommand(cmd);
}

void XraySource::setXrayOff()
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_XRAY_OFF;

    enqueueCommand(cmd);
}

void XraySource::setXrayKv(int kv)
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_XRAY_SET_KV;
    cmd->value = kv;

    enqueueCommand(cmd);
}

void XraySource::setXrayBeam(int beam)
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_XRAY_SET_BEAM;
    cmd->value = beam;

    enqueueCommand(cmd);
}

void XraySource::getXrayStatus(int interval)
{
    StXrayCommand *cmd = new StXrayCommand;
    cmd->type = XRAY_CMD_TYPE_XRAY_GET_STATUS;
    cmd->value = interval;

    enqueueCommand(cmd);
}

void XraySource::enqueueCommand(StXrayCommand *cmd)
{
    QMutexLocker locker(&s_mutex);
    m_commandQueue.enqueue(cmd);
}

