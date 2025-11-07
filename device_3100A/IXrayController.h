#ifndef IXRAYCONTROLLER_H
#define IXRAYCONTROLLER_H

#include <QObject>
#include <QString>

/**
 * @brief X射线控制器硬件抽象层接口
 *
 * 定义了 C3100AMotionWidget 与具体 X 射线源 (如 C3100AXrayController)
 * 之间的通信契约。
 */
class IXrayController : public QObject
{
    Q_OBJECT

public:
    // 虚析构函数，确保派生类被正确销毁
    virtual ~IXrayController() {}

    /**
     * @brief 连接到 X 射线源
     * @param portName 串口名称 (例如 "COM1")
     * @return true 连接成功, false 连接失败
     */
    virtual bool connectDevice(QString portName) = 0;

    /**
     * @brief 打开 X 射线
     */
    virtual void setXrayOn() = 0;

    /**
     * @brief 关闭 X 射线
     */
    virtual void setXrayOff() = 0;

    /**
     * @brief 设置目标电压 (kV)
     * @param kv 目标电压
     */
    virtual void setVoltage(int kv) = 0;

    /**
     * @brief 设置目标电流 (uA)
     * @param ua 目标电流
     */
    virtual void setCurrent(int ua) = 0;

signals:
    /**
     * @brief 状态更新信号
     * @param warmup 是否正在预热 (对应同事代码中的 warmingupState == 1)
     * @param locked 是否处于互锁 (Interlock) 状态
     * @param on X射线是否已打开
     * @param kv 当前实际电压
     * @param ua 当前实际电流
     */
    void sig_statusUpdated(bool warmup, bool locked, bool on, int kv, int ua);

    /**
     * @brief 连接状态改变信号
     * @param isConnected true为已连接, false为已断开
     */
    void sig_connected(bool isConnected);
};

#endif // IXRAYCONTROLLER_H
