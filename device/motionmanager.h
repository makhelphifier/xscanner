#ifndef MOTIONMANAGER_H
#define MOTIONMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "motioncommand.h"

class MotionCtrlCommand;
class MotionManager : public QObject
{
    Q_OBJECT
    friend class TopToolbar;
public:
    static MotionManager *getInstance();

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

    /**
     * @brief clearAxisErrcode   清除错误
     * @return
     */
    void clearAxisErrcode();

    void writeOutbit(ushort bitNo, ushort out);

    void setDoStatus(ushort bitNo);

    const MotionCtrlCommand* getCtrlCommand() const;

    void ReleaseIsRun();

    void beamStepShootFinished();

    void beamShootFinished();

    void beamContiShootFinished();//旋转dd马达 停止

    void beamContinuousScanning(double degSpeed,double start,double angleRange,float step);
    void beamStepScanning(int,int);
    void beamEmsStop();

    void allaxisHoming();
    double getAnnulusWPos();

    void scan();

    void one_btn_clicked();
    void double_btn_clicked();

    void autoReset();//自动流程复位

    void claming_btn_clicked();

    void mau_btn_clicked();
    void auto_btn_clicked();

    void leftdown();

    void rightdown();

    void upper_material_W_Pos(int index,double speed, double startAngle, double stopAngle, float stepLength);


    void SlipRingBackToStartingPos(double startPos);//滑环回到起始点
private:

    explicit MotionManager(QObject *parent = nullptr);

    ~MotionManager();

    void connections();

signals:
    void sig_resetTime(int value);

    void sig_connect(bool);

    void sig_dataChanged(const QVariant &var);

    void sig_DIStatusdataChanged(const QVariant &var);

    void sig_boardReset();

    void sig_softReset();

    void sig_ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode);

    void sig_jogMove(ushort axisNo, double vel, ushort dir);

    void sig_axHomingMove(ushort axisNo);

    void sig_axEnable(ushort axisNo);

    void sig_axDisenable(ushort axisNo);

    void sig_axisStop(ushort axisNo, ushort stopMode);

    void sig_emgStop();

    void sig_beamContinuousScanning(double degSpeed,double start,double angleRange,float step);

    void sig_releaseIsRun();

    void sig_beamContiScanningStart();

    void sig_beamContiScanningEnd();

    void sig_allowBeamShooting();

    void sig_beamContiShootFinished();

    void sig_beamStepScanningStart();

    void sig_beamStepScanningEnd();

    void sig_beamStepShootFinished();

    void sig_beamShootFinished();

    void sig_beamStepScanning(int degRange,int step);

    void sig_beamEmsStop();
    void sig_allaxisHoming();

    void sig_scan();

    void sig_one();
    void sig_double();

    void sig_autoReset();//自动流程复位

    void sig_claming_loose();

    void sig_leftdown();
    void sig_rightdown();

    void sig_ClearAxisErrcode();

    void sig_setDoStatus(ushort bitNo);
    void sig_writeOutbit(ushort bitNo, ushort out);

    void sig_rightRobotFeedingStepsRun1();//上料步骤1
    void sig_rightRobotFeedingStepsRun2();//上料步骤2
    void sig_rightRobotFeedingStepsRun3();//上料步骤3
    void sig_rightRobotFeedingStepsRun4();//上料步骤4
    void sig_rightRobotFeedingStepsRun5();//上料步骤5

    void sig_rightRobotBlankingStepsRun1();//下料步骤1
    void sig_rightRobotBlankingStepsRun2();//下料步骤2
    void sig_rightRobotBlankingStepsRun3();//下料步骤3

    void sig_rightStart(int index);//右侧按钮按下
    void sig_leftStart(int index);//左侧按钮按下

    void sig_rightRobotFeeding();//上料动作
    void sig_rightRobotBlanking();//下料动作
    void sig_rightInitialBitPos();//初始化

    void sig_rightDoubleFeedingStepRun1();
    void sig_rightDoubleFeedingStepRun2();
    void sig_rightDoubleFeedingStepRun3();
    void sig_rightDoubleFeedingStepRun4();


    void sig_leftInitialBitPos();//初始位置

    void sig_leftRobotFeedingStepsRun1();//上料步骤1

    void sig_leftRobotFeedingStepsRun2();//上料步骤2

    void sig_leftRobotFeedingStepsRun3();//上料步骤3

    void sig_leftRobotFeedingStepsRun4();//上料步骤4

    void sig_leftRobotFeedingStepsRun5();//上料步骤5


    void sig_leftRobotFeeding();//上料动作


    void sig_leftRobotBlankingStepsRun1();//上料步骤1

    void sig_leftRobotBlankingStepsRun2();//上料步骤2

    void sig_leftRobotBlankingStepsRun3();//上料步骤3

    void sig_leftRobotBlanking();//下料动作


    void sig_leftDoubleFeedingStepRun1();

    void sig_leftDoubleFeedingStepRun2();

    void sig_leftDoubleFeedingStepRun3();

    void sig_leftDoubleFeedingStepRun4();

    void sig_RightOneRun();//整体单电芯逻辑
    void sig_RightDoubleRun();//整体双电芯逻辑


    void sig_LeftOneRun();//整体单电芯逻辑
    void sig_LeftDoubleRun();//整体双电芯逻辑

    //到达检测位置
    void sig_ArrivalDetectionPos();

    void sig_upper_material_W_Pos(int index,double speed, double startAngle, double stopAngle, float stepLength);

    void sig_motionMsg(QString );

    void sig_mau();//手动
    void sig_auto();//自动


    void sig_SlipRingBackToStartingPos(double startPos);//滑环回到起始点

    //滑环运动完成
    void sig_TheSlipRingMovemenIsCompleted();


private:

    QThread m_thread;
    static QMutex m_Mutex;
    static MotionManager *sm_pInstance;

    MotionCtrlCommand* mcCommand;

    bool releaseFlag = true;

};

#endif // MOTIONMANAGER_H
