#ifndef XRAYSOURCE_H
#define XRAYSOURCE_H

#include <QObject>
#include <QMutex>
#include <QFuture>
#include <QQueue>

enum XrayCommandType {
    XRAY_CMD_TYPE_NONE = 0,
    XRAY_CMD_TYPE_CONNECT,
    XRAY_CMD_TYPE_DISCONNECT,
    XRAY_CMD_TYPE_XRAY_ON,
    XRAY_CMD_TYPE_XRAY_OFF,
    XRAY_CMD_TYPE_XRAY_SET_KV,
    XRAY_CMD_TYPE_XRAY_SET_BEAM,
    XRAY_CMD_TYPE_XRAY_GET_STATUS
};

struct StXrayCommand {
    XrayCommandType type = XRAY_CMD_TYPE_NONE;
    int value = 0;
};

class SerialComunicator;
class XraySource : public QObject
{
    Q_OBJECT
public:
    // Singleton instance access
    static XraySource* getInstance();
    static void destroyInstance();

    void startToConnect(int index);
    void stopToConnect();
    void setXrayOn();
    void setXrayOff();
    void setXrayKv(int kv);
    void setXrayBeam(int beam);
    void getXrayStatus(int interval);

private:
    explicit XraySource(QObject *parent = nullptr);
    ~XraySource();

    // Disable copy constructor and assignment operator
    XraySource(const XraySource&) = delete;
    XraySource& operator=(const XraySource&) = delete;

signals:
    void sig_didGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua);
    void sig_didConnect(int state);
    void sig_didDisconnect(int state);
    void sig_didLoadPortsList(const QStringList& nameList);
    void sig_processCommands();
private:
    void enqueueCommand(StXrayCommand *cmd);
    void onProcessCommands();

    // Singleton instance
    static XraySource* s_instance;
    static QMutex s_mutex;

    QQueue<StXrayCommand*> m_commandQueue;
    QFuture<void> m_commandFuture;
    bool m_commandLoop = true;
    int m_gettingStatusInterval = 100;
    QThread* m_workerThread=nullptr;

    SerialComunicator* m_serialPortComm;
};

#endif // XRAYSOURCE_H
