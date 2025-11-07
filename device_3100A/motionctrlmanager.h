#ifndef MOTIONCTRLMANAGER_H
#define MOTIONCTRLMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "motionctrlcommand.h"

class MotionCtrlCommand;
class MotionCtrlManager : public QObject
{
    Q_OBJECT
    friend class TopToolbar;
public:
    struct DDStatus
    {
        qint64 timer;
        QString objectiveTable_X1Pos;
        QString objectiveTable_Y1Pos;
        QString Detector_RPos;
        QString Detector_WPos;
    };

    static MotionCtrlManager *getInstance();

    /**
     * @brief boardReset  硬件复位
     * @return
     */
    void boardReset();

    /**
     * @brief softReset  软件复位
     * @return
     */
    void softReset();

    /**
     * @brief ptpMove   点位移动
     * @param axisNo    轴号
     * @param vel       速度
     * @param dist      位置
     * @param pos_mode  0-相对移动 1-绝对移动
     * @return
     */
    void ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode);

    /**
     * @brief jogMove   连续移动
     * @param axisNo    轴号
     * @param vel       速度
     * @param dir       方向  0-反向 1-正向
     * @return
     */
    void jogMove(ushort axisNo, double vel, ushort dir);

    /**
     * @brief axHomeMove   回零
     * @param axisNo     轴号
     * @return
     */
    void axHomingMove(ushort axisNo);

    /**
     * @brief axEnable    使能
     * @param axisNo    轴号
     * @return
     */
    void axEnable(ushort axisNo);

    /**
     * @brief disenable 下使能
     * @param axisNo    轴号
     * @return
     */
    void axDisenable(ushort axisNo);

    /**
     * @brief axisStop   单轴停止
     * @param axisNo    轴号
     * @param stopMode  急停模式    0-减速停止 1-紧急停止
     * @return
     */
    void axisStop(ushort axisNo, ushort stopMode);

    /**
     * @brief emgStop   急停(所有轴)
     * @return
     */
    void emgStop();

    void clearAxisErrcode();

    const MotionCtrlCommand* getCtrlCommand() const;

    void ReleaseIsRun();

    void beamStepShootFinished();


    void beamShootFinished();

    void beamContiShootFinished();//旋转dd马达 停止

    void beamContinuousScanning(float,double angleRange );
    void beamStepScanning(float,double);
    void beamEmsStop();

    void batteryClampRotates(float angle);


    void slotDDAxisPos(QVector<DDStatus>);

    void ptpMoveXY();

private:

    explicit MotionCtrlManager(QObject *parent = nullptr);

    ~MotionCtrlManager();

    void connections();

signals:
    void sig_resetTime(int value);

    void sig_connect(bool);

    void sig_dataChanged(const QVariant &var);

    void sig_boardReset();

    void sig_softReset();

    void sig_ptpMoveXY();


    void sig_ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode);

    void sig_jogMove(ushort axisNo, double vel, ushort dir);

    void sig_axHomingMove(ushort axisNo);

    void sig_axEnable(ushort axisNo);

    void sig_axDisenable(ushort axisNo);

    void sig_axisStop(ushort axisNo, ushort stopMode);

    void sig_emgStop();

    void sig_didMoveToAngle();

    void sig_doorStatusChanged(bool isopen);

    void sig_beamContinuousScanning(double speed,double angleRange);

    void sig_releaseIsRun();

    void sig_beamContiScanningStart();

    void sig_beamContiScanningEnd();

    void sig_allowBeamShooting();

    void sig_beamContiShootFinished();

    void sig_beamShootFinished();


    void sig_beamStepScanningStart();

    void sig_beamStepScanningEnd();

    void sig_beamStepShootFinished();

    void sig_calibratingStart();

    void sig_calibratingEnd();

    void sig_beamStepScanning(float degRange,double step);

    void sig_beamEmsStop();

    void sig_batteryClampRotates(int agleCount);

    void sig_ClearAxisErrcode();

    void sig_shoot();//到达检测位拍摄

public:

    ushort axisNo1;//光管 Z 轴
    ushort axisNo2;//探测源 X 轴
    ushort axisNo3;//探测源 Y 轴
    ushort axisNo4;//探测源 Z 轴
    ushort axisNo5;//旋转平台 D-X轴
    ushort axisNo6;//旋转平台 U-X轴
    ushort axisNo7;//旋转平台 Y 轴
    ushort axisNo8;//滤波片旋转 W 轴
    ushort axisNo9;//旋转轴
    ushort axisNo10;//旋转平台 Y 轴
    ushort axisNo11;//滤波片旋转 W 轴
    ushort axisNo12;//旋转轴
    ushort axisNo13;//旋转轴
    ushort axisNo14;//旋转轴
    ushort axisNo15;//旋转轴
    MotionCtrlCommand* mcCommand;

    bool DDposFlag = false;
    bool DDposFlagsave = false;

    QString fileDDPos ;
    DDStatus ddStatus;
    QVector<DDStatus> vecDDstatus;

private:

    QThread m_thread;
    static QMutex m_Mutex;
    static MotionCtrlManager *sm_pInstance;


};

#endif // MOTIONCTRLMANAGER_H
