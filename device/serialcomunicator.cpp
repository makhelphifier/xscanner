#include "serialcomunicator.h"
#include <QVariant>
// #include <QSerialPortInfo>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTimer>

bool portNameCompare(StSerialPortInfo *item1, StSerialPortInfo *item2)
{
    QString portName1 = item1->portName;
    QString portName2 = item2->portName;
    return portName1.replace("com", "", Qt::CaseInsensitive).toInt() < portName2.replace("com", "", Qt::CaseInsensitive).toInt();
}

SerialComunicator::SerialComunicator(QObject *parent)
    : QObject{parent}
{
    // m_updateStsCount = m_updateInterval;
    m_pStatusTimer = new QTimer(this);
    connect(m_pStatusTimer, &QTimer::timeout, this, [this](){
        getStatus();
        getInterLockStatus();
    });
    m_pStatusTimer->start(300);
}

SerialComunicator::~SerialComunicator()
{
    if (!m_itemList.isEmpty()) {
        qDeleteAll(m_itemList);
        m_itemList.clear();
    }
}

void SerialComunicator::loadPorts()
{
    // 清理现有端口列表
    qDeleteAll(m_itemList);
    m_itemList.clear();

    // 获取可用端口
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        StSerialPortInfo *item = new StSerialPortInfo();

        // 填充端口信息
        item->portName = info.portName();
        item->systemLocation = info.systemLocation();
        item->description = info.description();
        item->manufacturer = info.manufacturer();
        item->serialNumber = info.serialNumber();
        item->vendorIdentifier = info.vendorIdentifier();
        item->productIdentifier = info.productIdentifier();
        item->isNull = info.isNull();
        // item->isBusy = info.isBusy();

        m_itemList.append(item);
    }

    // 按COM口号排序
    std::sort(m_itemList.begin(), m_itemList.end(), portNameCompare);

    // 调试输出
    for(int i = 0; i < m_itemList.count(); i++) {
        qDebug() << "portName: " << m_itemList.at(i)->portName;
    }

    // 发送端口加载完成信号
    emit sig_didLoadPortList(m_itemList);
}


void SerialComunicator::selPort(int idx)
{
    // 检查索引是否有效
    if (idx < 0 || idx > m_itemList.count()-1)
        return;

    // 关闭当前可能打开的串口
    QSerialPort *serialPort = currSerialPortInst();
    if (serialPort->isOpen()) {
        serialPort->clear();
        serialPort->close();
    }

    // 设置当前端口索引
    m_currPortIndex = idx;

    // 配置串口参数
    StSerialPortInfo *item = m_itemList.at(idx);
    QSerialPort *sp = currSerialPortInst();

    // 设置串口通信参数
    sp->setBaudRate(QSerialPort::Baud38400);
    sp->setParity(QSerialPort::NoParity);
    sp->setDataBits(QSerialPort::Data8);
    sp->setStopBits(QSerialPort::OneStop);
    sp->setFlowControl( QSerialPort::NoFlowControl);

    // 设置串口名称和描述
    sp->setPortName(item->portName);
    sp->setProperty("description", QVariant(item->description));

    // 开始连接
    startConnecting();

}

bool SerialComunicator::isConnecting()
{
    QSerialPort *sp = currSerialPortInst();
    return sp->isOpen();
}

void SerialComunicator::startConnecting()
{
    QSerialPort *sp = currSerialPortInst();
    if (!sp->open(QSerialPort::ReadWrite)) {
    } else {
        getStatus();
    }
}

void SerialComunicator::stopConnecting()
{
    xrayOff();

    QSerialPort *sp = currSerialPortInst();
    if (sp->isOpen()) {
        sp->clear();
        sp->close();

        if (m_currPortIndex < 0 || m_currPortIndex > m_itemList.count()-1) {
            emit sig_didDisconnect(-1, "断开连接失败");
            return;
        }

        StSerialPortInfo *item = m_itemList.at(m_currPortIndex);
        item->xrayInfo = StXrayInfo();

        m_currPortIndex = -1;
    }
    emit sig_didDisconnect(1, "断开连接成功");
}

bool SerialComunicator::isXrayOn()
{
    if (m_currPortIndex < 0 || m_currPortIndex > m_itemList.count()-1)
        return false;

    StSerialPortInfo *item = m_itemList.at(m_currPortIndex);
    return item->xrayInfo.xRayOn;
}

/*
// get xray status
int SerialPortViewModel::xrayStatus()
{
//    XRAY
//    ! XRAY OFF
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write("XRAY");
    return 0;
}
*/

int SerialComunicator::xrayOn()
{
    //    XON
    //    Starts X-ray emission.
    int result = sendCommand("XON");
    if (result < 0) {
    }
    return result;
}

int SerialComunicator::xrayOff()
{
    //    XOF
    //    Stops X-ray emission.
    int result = sendCommand("XOF");
    if (result < 0) {
    }
    return result;
}

int SerialComunicator::setKv(int val)
{
    //    HIV 50
    //    Sets the X-ray tube voltage. Setting range is
    //    from 0 to 110. (Notes 1&2)
    QString str = QString("HIV %1").arg(val);
    int result = sendCommand(str);
    if (result < 0) {
    }
    return result;
}

/*
int SerialPortViewModel::hv()
{
//    HV
//    ! HV Measured 70.2 KV
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write("HV");
    return 0;
}
*/

int SerialComunicator::setBeam(int val)
{
    //    CUR 50
    //    Sets the X-ray tube current. Setting range is
    //    from 0 to 800. (Notes 1&2)
    QString str = QString("CUR %1").arg(val);
    int result = sendCommand(str);
    if (result < 0) {
    }
    return result;
}

/*
int SerialPortViewModel::beam()
{
//    BEAM
//    ! Beam measured 49.9 uA
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write("BEAM");
    return 0;
}
*/




int SerialComunicator::getStatus()
{
    //    SAR
    //    Example:  If the status is "XON" and the output
    //              is 50 kV, 30 μA, then the following response is
    //    returned: "SAR 3 50 30 0 0 0 0"
    return sendCommand("SAR");
}

int SerialComunicator::getInterLockStatus()
{
    //    SIN
    //    Returns the interlock status.
    //    "SIN 1": Interlock circuit is open.
    //    "SIN 0": Interlock is closed.

    return sendCommand("SIN");
}

/*
int SerialPortViewModel::program()
{
//    PROGRAM
//    ! Program Running 5
//    PROGRAM
//    ! Program Idle
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write("PROGRAM");
    return 0;
}

int SerialPortViewModel::programN(int n)
{
//    PROGRAM 1
//    ! OK
    QString str = QString("PROGRAM %1").arg(n);
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write(str.toUtf8());
    return 0;
}

int SerialPortViewModel::programEnd()
{
//    PROGRAM END;
//    ! OK
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write("PROGRAM END");
    return 0;
}


int SerialPortViewModel::parameters()
{
//    PARAMETERS
//    ! Parameters HV 20 to 130 Beam 0 to 500
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;
    sp->write("PARAMETERS");
    return 0;
}
*/

const QList<StSerialPortInfo*> &SerialComunicator::avaliblePorts()
{
    return m_itemList;
}

int SerialComunicator::currPortIndex()
{
    return m_currPortIndex;
}

StSerialPortInfo *SerialComunicator::currentItem()
{
    if (m_currPortIndex < 0 || m_currPortIndex > m_itemList.count()-1)
        return nullptr;

    return m_itemList.at(m_currPortIndex);
}


void SerialComunicator::standardBautRates()
{
    //    QList<qint32> baudRates = QSerialPortInfo::standardBaudRates();
}

QSerialPort *SerialComunicator::currSerialPortInst()
{
    if (m_pSerialPort == nullptr) {
        m_pSerialPort = new QSerialPort(this);
        connect(m_pSerialPort, &QSerialPort::baudRateChanged, this, [this](qint32 baudRate, QSerialPort::Directions directions){
            qDebug()<<"QSerialPort::baudRateChanged"<<baudRate<<", "<<directions;
        });
        connect(m_pSerialPort, &QSerialPort::dataBitsChanged, this, [this](QSerialPort::DataBits dataBits){
            qDebug()<<"QSerialPort::dataBitsChanged";
        });
        connect(m_pSerialPort, &QSerialPort::parityChanged, this, [this](QSerialPort::Parity parity){
            qDebug()<<"QSerialPort::parityChanged";
        });
        connect(m_pSerialPort, &QSerialPort::stopBitsChanged, this, [this](QSerialPort::StopBits stopBits){
            qDebug()<<"QSerialPort::stopBitsChanged";
        });
        connect(m_pSerialPort, &QSerialPort::flowControlChanged, this, [this](QSerialPort::FlowControl flowControl){
            qDebug()<<"QSerialPort::flowControlChanged";
        });
        connect(m_pSerialPort, &QSerialPort::dataTerminalReadyChanged, this, [this](bool set){
            qDebug()<<"QSerialPort::dataTerminalReadyChanged";
        });
        connect(m_pSerialPort, &QSerialPort::requestToSendChanged, this, [this](bool set){
            qDebug()<<"QSerialPort::requestToSendChanged";
        });
        connect(m_pSerialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error){

            QString msg = "";
            switch(error) {
            case QSerialPort::NoError:
                break;
            case QSerialPort::DeviceNotFoundError:
                msg = "DeviceNotFoundError";
                break;
            case QSerialPort::PermissionError:
                msg = "PermissionError";
                break;
            case QSerialPort::OpenError:
                msg = "OpenError";
                break;
            // case QSerialPort::ParityError:
            //     msg = "ParityError";
            //     break;
            // case QSerialPort::FramingError:
            //     msg = "FramingError";
            //     break;
            // case QSerialPort::BreakConditionError:
            //     msg = "BreakConditionError";
            //     break;
            // case QSerialPort::WriteError:
            //     msg = "WriteError";
            //     break;
            case QSerialPort::ReadError:
                msg = "ReadError";
                break;
            case QSerialPort::ResourceError:
                msg = "ResourceError";
                break;
            case QSerialPort::UnsupportedOperationError:
                msg = "UnsupportedOperationError";
                break;
            case QSerialPort::UnknownError:
                msg = "UnknownError";
                break;
            case QSerialPort::TimeoutError:
                msg = "TimeoutError";
                break;
            case QSerialPort::NotOpenError:
                msg = "NotOpenError";
                break;
            default:
                break;
            }
            if (!msg.isEmpty()) {
                qDebug()<<"QSerialPort::errorOccurred: "<<msg;
                emit sig_didRecvCmdReportMsg(QString("QSerialPort Error: %1").arg(msg));
            }
        });
        connect(m_pSerialPort, &QSerialPort::breakEnabledChanged, this, [this](bool set){
            qDebug()<<"QSerialPort::breakEnabledChanged";
        });

        // IODevice singals
        connect(m_pSerialPort, &QSerialPort::readyRead, this, [this](){
            QSerialPort *port = currSerialPortInst();
            QByteArray byteArr = port->readAll();

            // qDebug()<<"QSerialPort::readyRead"<<byteArr;

            m_recvedStr.append(byteArr);

            // qDebug()<<"\nm_recvedStr: "<<m_recvedStr;

            while (m_recvedStr.contains("\r")) {
                int crPos = m_recvedStr.indexOf("\r");
                if (crPos >= 0) {
                    // 提取一条完整指令（不包含\r）
                    QString cmd = m_recvedStr.left(crPos);

                    // 解析这条指令
                    if (!cmd.isEmpty()) {
                        parseRecvedData(cmd);
                    }

                    // 从缓冲区移除已处理的指令（包括\r）
                    m_recvedStr.remove(0, crPos + 1);
                }
            }

            // 此时m_recvedStr中可能还有不完整的指令，等待下次readyRead时继续处理
        });
        connect(m_pSerialPort, &QSerialPort::channelReadyRead, this, [this](int channel){
            qDebug()<<"QSerialPort::channelReadyRead";
        });
        connect(m_pSerialPort, &QSerialPort::bytesWritten, this, [this](qint64 bytes){
            //            qDebug()<<"QSerialPort::bytesWritten"<<bytes<<", "<<m_pSerialPort->readAll();
        });
        connect(m_pSerialPort, &QSerialPort::channelBytesWritten, this, [this](int channel, qint64 bytes){
            qDebug()<<"QSerialPort::channelBytesWritten"<<bytes;
        });
        connect(m_pSerialPort, &QSerialPort::aboutToClose, this, [this](){
            qDebug()<<"QSerialPort::aboutToClose";
        });
        connect(m_pSerialPort, &QSerialPort::readChannelFinished, this, [this](){
            qDebug()<<"QSerialPort::readChannelFinished";
        });
    }
    return m_pSerialPort;
}

bool SerialComunicator::parseRecvedData(const QString &str)
{
    // qDebug()<<"m_currCmd: "<<m_currCmd<<", "<<str;

    // 简单命令的响应处理
    if (str.contains("ERR", Qt::CaseInsensitive)) {
        return false;
    }
    if (str.contains("XON", Qt::CaseInsensitive)) {
        return true;
    }

    if (str.contains("XOF", Qt::CaseInsensitive)) {
        return true;
    }

    // 带参数命令的响应处理
    QStringList strList = str.split(" ");

    if (str.contains("HIV", Qt::CaseInsensitive) && strList.count() > 1) {
        bool ok;
        int value = strList.at(1).toInt(&ok);
        if (ok) {
        }
        return true;
    }

    if (str.contains("CUR", Qt::CaseInsensitive) && strList.count() > 1) {
        bool ok;
        int value = strList.at(1).toInt(&ok);
        if (ok) {
        }
        return true;
    }

    if (str.contains("SIN", Qt::CaseInsensitive) && strList.count() > 1) {
        //            qDebug()<<"SerialPortViewModel::parseRecvedData SIN: "<<str;
        int sts = strList.at(1).toInt();
        StSerialPortInfo *item = currentItem();
        if (item) {
            item->xrayInfo.interLock = !(sts == 0); // the door is closed when sts=0
            //                qDebug()<<"item->xrayInfo.interLock: "<<item->xrayInfo.interLock;
        }
        return true;
    }

    if (str.contains("SAR", Qt::CaseInsensitive) && strList.count() > 7) {
        //    ! Status On HV 70.2 070.0 BEAM 49.9 0050 Safe Infocus Spot 7

        //    SAR 0-6 0-110 0-800 0 0 0 0 (Parameters acquired in response to "STS",
        //      "SHV" and "SCU" commands are returned at one time, followed by 4 zeros (0) which are separated by a space.)
        //    STS 0: WARMUP YET
        //    STS 1: WARMUP
        //    STS 2: STANDBY
        //    STS 3: XON
        //    STS 4: OVER
        //    STS 5: PREHEA/INTER LOCK OFF/ERR *
        //    STS 6: SELF TEST


        //        Returns the X-ray source operation status.
        //        Response is returned with the following priority.
        //        "STS 5" : X-rays cannot be emitted.
        //        (Preheating, PC board defect, interlock open)
        //        "STS 4" : Overload protection is activated.
        //        "STS 1" : Warm-up in progress.
        //        "STS 6" : Self-test in progress.
        //        "STS 3" : X-rays are being emitted.
        //        "STS 0" : Awaiting warm-up.
        //        "STS 2" : Ready to emit X-rays.

        updateXrayStatus(strList);
        return true;
    }

    return false;
}


void SerialComunicator::updateXrayStatus(const QStringList &strList)
{
    // 检查参数有效性
    if (strList.count() < 4) {
        return;
    }

    StSerialPortInfo *item = currentItem();
    if (!item)
        return;

    // 重置状态
    item->xrayInfo.isReady = false;
    item->xrayInfo.xRayOn = false;
    item->xrayInfo.warmingupState = 2;
    item->xrayInfo.interLock = false;

    // 解析状态码
    bool ok;
    int sts = strList.at(1).toInt(&ok);
    if (!ok) return;

    // 根据状态码更新X射线状态
    switch(sts) {
    case 0: // WARMUP YET: Awaiting warm-up
        item->xrayInfo.warmingupState = 0;
        break;
    case 1: // WARMUP: Warm-up in progress
        item->xrayInfo.warmingupState = 1;
        break;
    case 2: // STANDBY: Ready to emit X-rays
        item->xrayInfo.isReady = true;
        break;
    case 3: // XON: X-rays are being emitted
        item->xrayInfo.xRayOn = true;
        break;
    case 4: // OVER: Overload protection is activated
        break;
    case 5: // PREHEA/INTER LOCK OFF/ERR *: X-rays cannot be emitted
        item->xrayInfo.interLock = true;
        break;
    case 6: // SELF TEST: Self-test in progress
        break;
    default:
        break;
    }

    // 更新电压和电流值
    int kv = strList.at(2).toInt(&ok);
    if (ok) {
        item->xrayInfo.currKv = kv;
    }

    int ua = strList.at(3).toInt(&ok);
    if (ok) {
        item->xrayInfo.currUA = ua;
    }


    // 发送当前项更新信号
    emit sig_didGetStatus(item);
}

int SerialComunicator::sendCommand(const QString &cmdStr)
{
    QSerialPort *sp = currSerialPortInst();
    if (!sp->isOpen())
        return -1;

    m_cmdList << cmdStr;

    QString str = QString("%1\r").arg(cmdStr);
    QByteArray data = str.toUtf8();
    qint64 bytesWritten = sp->write(data);

    if (bytesWritten != data.size()) {
        qDebug() << "Failed to write all data. Written:" << bytesWritten << "Expected:" << data.size();
        return -2;
    }

    if (!sp->waitForBytesWritten(5000)) {
        qDebug() << "Write operation timed out";
        return -3;
    }

    return 0;
}

