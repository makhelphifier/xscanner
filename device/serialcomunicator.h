#ifndef SERIALCOMUNICATOR_H
#define SERIALCOMUNICATOR_H

#include <QObject>
#include <QDebug>
// #include <QSerialPort>
#include <QtSerialPort/QSerialPort>
class QTimer;

struct StXrayInfo {
    uchar warmingupState = 0; // 0 not warm up yet, 1 is warming up, 2 warm up finished
    bool xRayOn = false;
    bool isReady = false;
    int currKv = 0;
    int currUA = 0;
    bool interLock = false;
};

struct StSerialPortInfo{
    quint16 vendorIdentifier = 0;
    quint16 productIdentifier = 0;

    QString portName = "";
    QString systemLocation = "";
    QString description = "";
    QString manufacturer = "";
    QString serialNumber = "";

    bool isNull = false;
    bool isBusy = false;

    StXrayInfo xrayInfo;
    void printSelf() {
        qDebug()<<"portName: "<<portName;
        qDebug()<<"warmingupState: "<<xrayInfo.warmingupState;
        qDebug()<<"xRayOn: "<<xrayInfo.xRayOn;
        qDebug()<<"isReady: "<<xrayInfo.isReady;
        qDebug()<<"currKv: "<<xrayInfo.currKv;
        qDebug()<<"currUA: "<<xrayInfo.currUA;
        qDebug()<<"interLock: "<<xrayInfo.interLock;
    }

};
Q_DECLARE_METATYPE(StSerialPortInfo);

class SerialComunicator : public QObject
{
    Q_OBJECT
public:
    explicit SerialComunicator(QObject *parent = nullptr);
    ~SerialComunicator();

    void loadPorts();
    void selPort(int idx);

    bool isConnecting();
    void startConnecting();
    void stopConnecting();

    bool isXrayOn();
    int xrayOn();
    int xrayOff();
    int setKv(int val);
    int setBeam(int val);

    int getStatus();
    int getInterLockStatus();

    const QList<StSerialPortInfo*>& avaliblePorts();
    int currPortIndex();

    StSerialPortInfo *currentItem();
signals:
    void sig_didSelectPort(const QString& msg);
    void sig_didRecvCmdReportMsg(const QString& msg);
    void sig_didConnect(int state, const QString& msg);
    void sig_didDisconnect(int state, const QString& msg);
    void sig_didGetParameters(const QHash<QString, int>& params);

    void sig_didLoadPortList(const QList<StSerialPortInfo*>& itemList);
    void sig_didGetStatus(StSerialPortInfo *item);

    void sig_xrayOnSuccess(bool success, const QString& msg);
    void sig_xrayOffSuccess(bool success, const QString& msg);
    void sig_setKvSuccess(bool success, int value, const QString& msg);
    void sig_setBeamSuccess(bool success, int value, const QString& msg);

private:
    void standardBautRates();
    QSerialPort *currSerialPortInst();
    bool parseRecvedData(const QString &str);
    void updateXrayStatus(const QStringList &strList);

    int sendCommand(const QString& cmdStr);
private:
    QSerialPort *m_pSerialPort = nullptr;

    QStringList m_baudRateList;
    QStringList m_parityList;
    QStringList m_dataBitsList;
    QStringList m_stopBitsList;
    QStringList m_flowCtrlList;

    QList<StSerialPortInfo*> m_itemList;
    int m_currPortIndex = -1;

    QTimer *m_pStatusTimer = nullptr;

    QStringList m_cmdList;
    QString m_currCmd = "";
    QString m_recvedStr = "";
};

#endif // SERIALCOMUNICATOR_H
