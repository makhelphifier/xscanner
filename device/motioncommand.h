#ifndef MOTIONCTRLCOMMAND_H
#define MOTIONCTRLCOMMAND_H

#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>
#include <QtCore>
#include <QCoreApplication>
#include <QVariant>
#include <QPair>
#include <atomic>
#include "motiondef.h"

class MotionCtrlCommand : public QObject
{
    Q_OBJECT
private:
    explicit MotionCtrlCommand(QObject *parent = nullptr);
    ~MotionCtrlCommand();
public:
    enum TASKTYPE{
        TASK_NONE,
        TASK_CONTINUINGMOVE,
        TASK_CONTINUINGMOVE_HOME,
        TASK_CONTINUINGMOVE_START,
        TASK_CONTINUINGMOVE_SHOOT,
        TASK_CONTINUINGMOVE_RUNING
    };Q_ENUM(TASKTYPE);

    static MotionCtrlCommand *getInstance();
    /**
     * @brief boardReset  硬件复位
     * @return
     */
    bool boardReset();

    /**
     * @brief softReset  软件复位
     * @return
     */
    bool softReset();

    /**
     * @brief ptpMove   点位移动
     * @param axisNo    轴号
     * @param vel       速度
     * @param dist      位置
     * @param pos_mode  0-相对移动 1-绝对移动
     * @return
     */
    bool ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode);

    /**
     * @brief jogMove   连续移动
     * @param axisNo    轴号
     * @param vel       速度
     * @param dir       方向  0-反向 1-正向
     * @return
     */
    bool jogMove(ushort axisNo, double vel, ushort dir);

    /**
     * @brief homeMove   回零
     * @param axisNo     轴号
     * @return
     */
    bool axHomingMove(ushort axisNo);
    /**
     * @brief enable    使能
     * @param axisNo    轴号
     * @return
     */
    bool axEnable(ushort axisNo);

    /**
     * @brief disenable 下使能
     * @param axisNo    轴号
     * @return
     */
    bool axDisenable(ushort axisNo);

    /**
     * @brief axisStop   单轴停止
     * @param axisNo    轴号
     * @param stopMode  急停模式    0-减速停止 1-紧急停止
     * @return
     */
    bool axisStop(ushort axisNo, ushort stopMode);

    /**
     * @brief emgStop   急停(所有轴)
     * @return
     */
    bool emgStop();

    /**
     * @brief readInbit 读取输入口状态
     * @param bitNo     位编号
     * @param state     状态返回值
     * @return
     */
    bool readInbit(ushort bitNo, ushort *state);

    /**
     * @brief readOutbit    读取输出口状态
     * @param bitNo         位编号
     * @param state         状态返回值
     * @return
     */
    bool readOutbit(ushort bitNo, ushort *state);

    /**
     * @brief writeOutbit   设置输出口状态
     * @param bitNo         位编号
     * @param out           设置值 0-导通 1-断开
     * @return
     */
    bool writeOutbit(ushort bitNo, ushort out);

    /**
     * @brief writeOutbit   设置输出口状态
     * @param bitNo         位编号
     * @param out           设置值 0-导通 1-断开
     * @return
     */
    void setDoStatus(ushort bitNo);

    /**
     * @brief readParamInit 读取及设置各个轴的脉冲初始值
     * @param initFile  ini文件路径
     * @return
     */
    bool readParamInit(const QString &initFile);
    /**
     * @brief nmcClearAxisErrcode 清除总线轴错误码
     * @return
     */
    void nmcClearAxisErrcode();

    const int getCardNo() const;

    void updateMotionParams(const QString& configPath);

    void beamContinuousScanning(double degSpeed,double start,double angleRange,float step);//连续扫描

    void beamStepScanning(int degRange, int step);//步进扫描

    void readDDActPos(double start,double angleRange,float step);

    void slot_beamEmsStop();

    short lineHcmp(int increment);
    /*
    输入测量的每个绝对定位的位置、以及其对应的误差、电机运行的目标位置。输出修正后的电机目标位置。
    */
    float actLinearCompensationOfmotor(std::vector<std::vector<float>> & locationAndErr,float currLocation);

    double getAnnulusWPos();//获取圆环位置

    void SlipRingBackToStartingPos(double startPos);//滑环回到起始点

public:
    //回零完成
    bool axisHomingMoveFinish(ushort axisNo);

    void allaxisHoming();

public slots:
    void slot_startTimerTask();
    void slot_stopTimerTask();
    void slot_test();
    void slot_releaseIsRun();
    void slot_beamContiShootFinished();
    void slot_beamStepShootFinished();

private slots:
    //读取 EtherCAT 总线状态
    void readCntStatus();

    void readData();

    void readIOStatus();

signals:
    void sig_resetTime(int value);

    void sig_connect(bool);

    void sig_dataChanged(const QVariant &var);
    void sig_DIStatusdataChanged(const QVariant &var);


    void sig_moveAcqSpotComplete();

    void sig_moveLightToAhchorComplete();//12-13zb添加

    void sig_beginAcq();

    //允许拍摄信号
    void sig_allowBeamShooting();
    //步进扫描
    void sig_beamStepScanningStart();

    void sig_beamStepScanningEnd();

    //持续扫描
    void sig_beamContiScanningStart();
    void sig_beamContiScanningEnd();

    //到达检测位置
    void sig_ArrivalDetectionPos();

    void sig_motionMsg(QString);


    //滑环运动完成
    void sig_TheSlipRingMovemenIsCompleted();

private:
    bool connect();

    bool disconnect();

    bool init();

public:
    float dist0 = 0;
    float dist1 = 0;
    float dist4 = 0;


    QVector<MotionCtrlData::MotionCtrlInfo> mcInfoVec;

private:
    //读取ini文件数据
    // QString getValueFromIni(QString str);
    // void setValue2Ini(QString str, QString value);
    QString getValueFromAdaptorIni(QString str);
    // void setValue2AdaptorIni(QString str, QString value);

    QString iniPath;
    QSharedPointer<QSettings> configSharedPointer;

    QString adaptorIniPath;//adaptor
    QSharedPointer<QSettings> adaptorSharedPointer;//adaptor


    //线程任务
    bool isHaveTask = false;
    QFuture<void> taskFuture;
    bool isRun = true;

    //定时器
    QTimer *readCntStsTimer = nullptr;
    QTimer *readDataTimer = nullptr;
    QTimer *readIODataTimer = nullptr;

    bool isConnected = false;
    bool isCamScan = false;
    short res;
    ushort CurrCardNo;

    double batteryPos;
    QMap<QString,int> IoPortMap;//输出IO

    QMap<int,QPair<double,double>> pos_limits;

    std::atomic<bool> b_scanLoop;

    static QMutex m_Mutex;


    TASKTYPE tasktype = TASK_NONE;
    bool isShootFinished = false;
    double continueMoveSpeed = 0;
    qint64 startTime;
    bool isEmsStop = false;
    bool isStepRun = false;

    bool doorFlag = false;
    QMap<int,QPair<bool,bool>> pos_limitsFlag;//正负限位标志位

    // AxisFormula axisformula;

    // QVector<AxisFormula::TableRow> vecTableRow;
    bool vecTableRowFlag = false;

public:
    bool vecStartFlag = false;

    bool leftDet = false;
    bool rightDet = false;

    bool leftDetw = false;
    bool rightDetw = false;
    bool m_wPosFinish = false;


    bool m_one = false;
    bool m_double = true;

    bool m_mau = false;//手动
    bool m_auto = true;//自动


    bool m_leftDetectionFinish = false;//是否检测完成
    bool m_rightDetectionFinish = false;//是否检测完成
    bool m_leftupFinish = false;//是否到达上料位
    bool m_rightupFinish = false;//是否到达上料位

    int m_leftType = 0;//左侧模式
    int m_rightType = 0;//右侧模式

    bool m_leftClearStatu = false;//左侧清料模式
    bool m_rightClearStatu = false;//右侧清料模式


    bool stopFlag = true;//

    int m_mechanical_arm_Z = 0;//机械手升降轴 中转位置

    bool m_autoBatteryMemory = false; //自动模式电池记忆


    bool resetMove = false;//复位


    int m_annulus_W_index = 0 ;//如果转过3次角，并有检测完成信号就是真的检测完成
    bool m_beamShootFinished = false;//是否三圈结束，检测完成

    int m_count = 0 ;//超时之后停止

    int m_axisCount = 10;//轴数
};

#endif // MOTIONCTRLCOMMAND_H
//错误码：3100 正向硬件限位触发，不允许正向启动vmove
//错误码：3101 负向硬件限位触发，不允许负向启动vmove
//错误码：1012 总线控制卡或控制器轴未使能，先使能轴再操作
//错误码: 1801  IO输入输出口超出最大允许值（16）或映射轴号超出范围
