#include "device_3100A/C3100AXrayController.h"
#include <QTimer>
#include <QDebug>
#include "util/logger/logger.h" // 引入日志

C3100AXrayController::C3100AXrayController(QObject *parent)
    : IXrayController(),
    m_port(new QSerialPort(this)),
    m_pollTimer(new QTimer(this)),
    m_isWarmingUp(false),
    m_isLocked(true), // 默认互锁
    m_isXrayOn(false),
    m_currentKv(0),
    m_currentUa(0)
{
    // 连接串口信号
    connect(m_port, &QSerialPort::readyRead,
            this, &C3100AXrayController::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred,
            this, &C3100AXrayController::onErrorOccurred);

    // 连接定时器信号
    connect(m_pollTimer, &QTimer::timeout,
            this, &C3100AXrayController::onPollStatus);
}

C3100AXrayController::~C3100AXrayController()
{
    if (m_port->isOpen()) {
        m_pollTimer->stop();
        m_port->close();
    }
}

bool C3100AXrayController::connectDevice(QString portName)
{
    if (m_port->isOpen()) {
        m_port->close();
    }

    // 参考 SerialComunicator::selPort
    m_port->setPortName(portName);
    m_port->setBaudRate(QSerialPort::Baud38400);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (m_port->open(QIODevice::ReadWrite)) {
        LogInfo(QString("XrayController: Successfully connected to %1").arg(portName));
        // 启动定时轮询，参考 SerialComunicator 构造函数
        m_pollTimer->start(300);
        emit sig_connected(true);
        return true;
    } else {
        LogError_(QString("XrayController: Failed to open port %1. Error: %2")
                      .arg(portName)
                      .arg(m_port->errorString()));
        emit sig_connected(false);
        return false;
    }
}

void C3100AXrayController::setXrayOn()
{
    // 参考 SerialComunicator::xrayOn
    sendCommand("XON");
}

void C3100AXrayController::setXrayOff()
{
    // 参考 SerialComunicator::xrayOff
    sendCommand("XOF");
}

void C3100AXrayController::setVoltage(int kv)
{
    // 参考 SerialComunicator::setKv
    sendCommand(QString("HIV %1").arg(kv));
}

void C3100AXrayController::setCurrent(int ua)
{
    // 参考 SerialComunicator::setBeam
    sendCommand(QString("CUR %1").arg(ua));
}

void C3100AXrayController::onReadyRead()
{
    // 参考 SerialComunicator::readyRead
    m_readBuffer.append(m_port->readAll());

    while (m_readBuffer.contains('\r')) {
        int crPos = m_readBuffer.indexOf('\r');
        QString line = m_readBuffer.left(crPos).trimmed();
        m_readBuffer.remove(0, crPos + 1);

        if (!line.isEmpty()) {
            // LogDebug(QString("Xray Recv: %1").arg(line));
            parseLine(line);
        }
    }
}

void C3100AXrayController::onErrorOccurred(QSerialPort::SerialPortError error)
{
    // 发生错误，特别是资源错误，意味着断线
    if (error == QSerialPort::ResourceError) {
        LogError_(QString("XrayController: Port error (ResourceError). Disconnecting."));
        m_pollTimer->stop();
        m_port->close();

        // 重置状态
        m_isWarmingUp = false;
        m_isLocked = true;
        m_isXrayOn = false;
        m_currentKv = 0;
        m_currentUa = 0;

        emit sig_connected(false);
        emitStatusUpdate(); // 发送重置后的状态
    }
}

void C3100AXrayController::onPollStatus()
{
    // 参考 SerialComunicator 构造函数中的定时器
    if (m_port->isOpen()) {
        sendCommand("SAR"); // 获取状态
        sendCommand("SIN"); // 获取互锁状态
    }
}

void C3100AXrayController::sendCommand(const QString& cmd)
{
    if (!m_port->isOpen()) {
        LogError_(QString("XrayController: Port not open. Cannot send command: %1").arg(cmd));
        return;
    }

    // LogDebug(QString("Xray Send: %1").arg(cmd));
    QByteArray data = (cmd + "\r").toUtf8();
    m_port->write(data);
}

void C3100AXrayController::parseLine(const QString& line)
{
    // 参考 SerialComunicator::parseRecvedData
    QStringList parts = line.split(' ');

    if (line.contains("SAR") && parts.count() > 7) {
        // "SAR 3 50 30 0 0 0 0"
        updateStatusFromSAR(parts);
    } else if (line.contains("SIN") && parts.count() > 1) {
        // "SIN 1" (Open/Locked) or "SIN 0" (Closed)
        updateStatusFromSIN(parts);
    }
    // (可以忽略 XON, XOF, HIV, CUR 的 "OK" 响应)
}

void C3100AXrayController::updateStatusFromSAR(const QStringList& parts)
{
    // 参考 SerialComunicator::updateXrayStatus
    bool ok;
    int sts = parts.at(1).toInt(&ok);
    if (!ok) return;

    // 重置瞬时状态
    m_isWarmingUp = false;
    m_isXrayOn = false;
    // m_isLocked 状态由 SIN 独立管理，但 SAR 5 是错误状态也应视为锁定
    // m_isLocked = false; // 不重置，等待 SIN

    switch(sts) {
    case 0: // WARMUP YET
        break;
    case 1: // WARMUP
        m_isWarmingUp = true;
        break;
    case 2: // STANDBY
        break;
    case 3: // XON
        m_isXrayOn = true;
        break;
    case 4: // OVER
        break;
    case 5: // PREHEA/INTER LOCK OFF/ERR
        m_isLocked = true; // STS 5 强制互锁/错误状态
        break;
    case 6: // SELF TEST
        break;
    }

    m_currentKv = parts.at(2).toInt(&ok);
    m_currentUa = parts.at(3).toInt(&ok);

    emitStatusUpdate();
}

void C3100AXrayController::updateStatusFromSIN(const QStringList& parts)
{
    // 参考 SerialComunicator::parseRecvedData
    bool ok;
    int sts = parts.at(1).toInt(&ok);
    if (!ok) return;

    // "SIN 1": Interlock circuit is open. (Locked)
    // "SIN 0": Interlock is closed. (Safe)
    m_isLocked = (sts == 1);

    emitStatusUpdate();
}

void C3100AXrayController::emitStatusUpdate()
{
    // 统一发出信号
    emit sig_statusUpdated(m_isWarmingUp, m_isLocked, m_isXrayOn, m_currentKv, m_currentUa);
}
