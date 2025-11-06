#ifndef IMOTIONCONTROLLER_H
#define IMOTIONCONTROLLER_H

#include <QObject>
#include <QVariant> // 用于 sig_axisDataUpdated

/**
 * @brief 运动控制器硬件抽象层接口
 *
 * 定义了所有运动控制器必须实现的通用功能，
 * 隔离了上层逻辑与具体的硬件SDK实现
 *
 */
class IMotionController : public QObject
{
    Q_OBJECT

public:
    // 明确声明虚析构函数，确保正确的资源释放
    virtual ~IMotionController() {}

    // --- 1. 连接 / 初始化 ---

    /**
     * @brief 连接运动控制器设备
     * @return true 连接成功, false 连接失败
     */
    virtual bool connectDevice() = 0;

    /**
     * @brief 断开运动控制器设备
     */
    virtual void disconnectDevice() = 0;

    /**
     * @brief 硬件复位
     */
    virtual void boardReset() = 0;

    /**
     * @brief 软件复位
     */
    virtual void softReset() = 0;

    // --- 2. 轴控制 ---

    /**
     * @brief 使能或禁用单个轴
     * @param axisId 轴ID (由实现类映射到具体的轴号)
     * @param enable true为使能, false为禁用
     * @return true 操作成功, false 操作失败
     */
    virtual bool setAxisEnabled(int axisId, bool enable) = 0;

    /**
     * @brief PTP (点对点) 绝对位置移动
     * @param axisId 轴ID
     * @param position 目标绝对位置
     * @return true 命令发送成功, false 命令发送失败
     */
    virtual bool ptpMove(int axisId, double position) = 0;

    /**
     * @brief JOG (点动) 连续移动
     * @param axisId 轴ID
     * @param positive true为正向移动, false为反向移动
     * @return true 命令发送成功, false 命令发送失败
     */
    virtual bool jogMove(int axisId, bool positive) = 0;

    /**
     * @brief 停止单个轴的运动
     * @param axisId 轴ID
     */
    virtual void stopAxis(int axisId) = 0;

    /**
     * @brief 执行单个轴的回零(Homing)操作
     * @param axisId 轴ID
     */
    virtual void homeAxis(int axisId) = 0;

    // --- 3. 全局控制 ---

    /**
     * @brief 紧急停止 (停止所有轴)
     */
    virtual void stopAll() = 0;

    // --- 4. 状态查询 ---

    /**
     * @brief 检查指定轴是否已完成移动
     * @param axisId 轴ID
     * @return true 移动完成, false 正在移动
     * @note 推荐优先使用 sig_moveComplete 信号，而不是轮询。
     */
    virtual bool isMoveDone(int axisId) = 0;

signals:
    /**
     * @brief 轴移动完成信号
     * @param axisId 完成移动的轴ID
     */
    void sig_moveComplete(int axisId);

    /**
     * @brief 轴数据更新信号
     * @param axisData 包含所有轴状态的数据。
     * 使用 QVariant 是为了保持接口的通用性，
     * 具体实现可以传递一个 QVector<MotionCtrlData::MotionCtrlInfo>。
     */
    void sig_axisDataUpdated(const QVariant &axisData);

    /**
     * @brief 控制器连接状态改变信号
     * @param isConnected true为已连接, false为已断开
     */
    void sig_connected(bool isConnected);

    /**
     * @brief 轴回零完成信号
     * @param axisId 完成回零的轴ID
     */
    void sig_homingComplete(int axisId);
};

#endif // IMOTIONCONTROLLER_H
