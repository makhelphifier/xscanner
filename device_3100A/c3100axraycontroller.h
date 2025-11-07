#ifndef C3100AXRAYCONTROLLER_H
#define C3100AXRAYCONTROLLER_H

#include "device_3100A/IXrayController.h"
#include <QtSerialPort/QSerialPort>

class QTimer;

/**
 * @brief C3100A X 射线控制器的具体实现
 *
 * 继承 IXrayController 接口，
 * 内部使用 QSerialPort 和 QTimer 来管理与X射线源的通信。
 * 协议参考自 SerialComunicator。
 */
class C3100AXrayController : public IXrayController
{
    Q_OBJECT

public:
    explicit C3100AXrayController(QObject *parent = nullptr);
    ~C3100AXrayController();

    // --- IXrayController 接口实现 ---
    bool connectDevice(QString portName) override;
    void setXrayOn() override;
    void setXrayOff() override;
    void setVoltage(int kv) override;
    void setCurrent(int ua) override;

private slots:
    // 串口数据接收槽
    void onReadyRead();
    // 串口错误处理槽
    void onErrorOccurred(QSerialPort::SerialPortError error);
    // 定时器轮询槽
    void onPollStatus();

private:
    /**
     * @brief 发送命令到串口
     * @param cmd 命令字符串 (不含 \r)
     */
    void sendCommand(const QString& cmd);

    /**
     * @brief 解析从串口接收到的单行数据
     * @param line 不含 \r 的单行响应
     */
    void parseLine(const QString& line);

    /**
     * @brief 根据 "SAR" (Status All Report) 响应更新状态
     * @param parts "SAR" 响应按空格分割后的列表
     */
    void updateStatusFromSAR(const QStringList& parts);

    /**
     * @brief 根据 "SIN" (Status Interlock) 响应更新状态
     * @param parts "SIN" 响应按空格分割后的列表
     */
    void updateStatusFromSIN(const QStringList& parts);

    /**
     * @brief 统一发出状态更新信号
     */
    void emitStatusUpdate();

    QSerialPort* m_port;      // 串口实例
    QTimer* m_pollTimer; // 轮询定时器
    QString m_readBuffer;  // 串口读取缓冲区

    // 缓存的X射线机状态
    bool m_isWarmingUp;
    bool m_isLocked;
    bool m_isXrayOn;
    int m_currentKv;
    int m_currentUa;
};

#endif // C3100AXRAYCONTROLLER_H
