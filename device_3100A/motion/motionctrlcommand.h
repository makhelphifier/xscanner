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

#include "appconfig.h"
#include "axisformula.h"

namespace MotionCtrlData {
struct MotionCtrlInfo
{
    bool enable;//使能状态
    bool busy;//运动完成状态
    bool org;//原点
    bool almWarn;//伺服报警
    bool hardPosLmtWarn;//正限位
    bool hardNegLmtWarn;//负限位
    bool softPosLmtWarn;//正软限位
    bool softNegLmtWarn;//负软限位
    bool emgStop;//急停
    double axEncPos;//位置
    double axEncVel;//速度
    float curdist;
    bool ZeroingIsComplete = false;//回零状态
    int axisnum ;//轴号
    MotionCtrlInfo() {}

};

}Q_DECLARE_METATYPE(MotionCtrlData::MotionCtrlInfo);


enum CamPos
{
    POS_LOW,
    POS_HIGH
};

enum MachineStatus
{
    MS_NONE,
    MS_WARN,
    MS_ALARM
};

struct DIStatus
{
    ushort IO_Port;//端口
    ushort IO_Status;//状态
};
Q_DECLARE_METATYPE(DIStatus);


enum AXIS{
    objectiveTable_X1 = 0,
    objectiveTable_X2,
    objectiveTable_Y1,
    objectiveTable_Y2,
    Detector_Z1,
    Detector_Z2,
    XRAY_Z,
    Detector_R,
    Detector_W
};
struct axisPos08{
    double objectiveTableXPos = 0;//载物台x轴
    double objectiveTableYPos = 0;//载物台y轴
    double Detector_RPos = 0;//探测器 r轴
    double Detector_WPos = 0;//探测器 theta
};
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

    std::vector<std::vector<float>> csvParsing(QString );

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
     * @brief readParamInit 读取及设置各个轴的脉冲初始值
     * @param initFile  ini文件路径
     * @return
     */
    bool readParamInit(const QString &initFile);

    void HcmpSetMode(unsigned short cmp_logic = 0);//高速位置比较

    //清除总线轴错误 axis 轴号
    void nmcClearAxisErrcode();

    bool allAxisResetHoming(); //队列运行

    bool allAxisResetHoming2(); //多线程同时

    bool ligntAxisResetHoming();//光管进行回零

    bool moveAcqSpot();

    bool moveLightToAhchor();//光管移动到锚点

    void setCameraVelocity(double vel,int angleNo);

    void CamScan(bool loop = false,int angleNo = 1);

    void CamScan2(bool loop = false,int angleNo = 1); //微调后拍图

    void stopScan();

    void CamMove(CamPos pos,double startVel = 0,double vel = 0,double Tacc = 0.0001,double Tdec = 0.0001,double StopVel = 0.0);

    void setAngle(int angleNo);

    void retrieveBattry();

    void battryBack();

    const int getCardNo() const;

    void updateMotionParams(const QString& configPath);



    void openDoor();//开门


    void axisGetStopReason();//设置软限位


    void beamContinuousScanning(double degSpeed,double angleRange);//连续扫描
    void beamStepScanning(float degRange, double step);//步进扫描

    //=====================================================

    void beamStepScanning3100A(int degRange, int step);//步进扫描
    void beamStepScanningfinishShooting();//步进扫描
    // 计算公式
    double calculateFormula(double r, double sod, double d, double s);

    // 输入验证
    bool validateInput(double r, double sod, double d, double s);

    void slot_beamEmsStop3100A();
    //======================================================

    std::vector<float> csvAxis0Parsing(QString filename);//轴0  步进扫描步长

    void HcmpBeamContinuousScanning(double degSpeed);//连续扫描
    void HcmpBeeamStepScanning(int degRange, int step);//步进扫描
    void HcmpReadDDActPos();

    void readDDActPos(double angleRange);
    void slot_beamEmsStop();

    void slot_batteryClampRotates(float angleCount);

    bool axisAllHomingMove();//所有轴回零

    void axisDDToZero();
    /*
    输入测量的每个绝对定位的位置、以及其对应的误差、电机运行的目标位置。输出修正后的电机目标位置。
*/
    float actLinearCompensationOfmotor(std::vector<std::vector<float>> & locationAndErr,float currLocation);

    bool ptpMoveXY();


    void slot_beamShootFinished();

public:
    //pwm 输出
    void pwmOutput(double frame);

public:
    //回零完成
    bool axisHomingMoveFinish(ushort axisNo);

    void setPosLimit(QMap<int,QPair<bool,bool>>);

    QMap<int,QPair<bool,bool>>  getPosLimit();



    axisPos08 axisDDPos();

public slots:
    void slot_startTimerTask();
    void slot_stopTimerTask();
    void slot_test();
    void slot_releaseIsRun();
    void slot_beamContiShootFinished();
    void slot_beamStepShootFinished();
private slots:

    void readCntStatus();

    void readData();

    void readIOStatus();

signals:
    void sig_resetTime(int value);

    void sig_connect(bool);

    void sig_dataChanged(const QVariant &var);
    void sig_DIStatusdataChanged(const QVariant &var);

    void sig_switchAngleComplete();
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

    void sig_shoot();//到达检测位拍摄
private:
    bool connect();

    bool disconnect();

    bool init();

    void setParams();

public:

    ushort axisNo1 = 0; //轴1
    ushort axisNo2 = 1; //轴2
    ushort axisNo3 = 2; //轴3
    ushort axisNo4 = 3; //轴4
    ushort axisNo5 = 4; //轴5
    ushort axisNo6 = 5; //轴6
    ushort axisNo7 = 6; //轴7
    ushort axisNo8 = 7; //轴8
    ushort axisNo9 = 8; //轴9
    ushort axisNo10 = 9; //轴10
    ushort axisNo11 = 10; //轴11
    ushort axisNo12 = 11; //轴12
    ushort axisNo13 = 12; //轴13
    ushort axisNo14 = 13; //轴12
    ushort axisNo15 = 14; //轴13

    QVector<MotionCtrlData::MotionCtrlInfo> mcInfoVec;
    float dist0 = 0;
    float dist1 = 0;
    float dist4 = 0;
private:
    //读取ini文件数据
    QString getValueFromIni(QString str);
    void setValue2Ini(QString str, QString value);

    QString getValueFromAdaptorIni(QString str);
    void setValue2AdaptorIni(QString str, QString value);

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

    double axisAnchor1;
    double axisAnchor2;
    double axisAnchor3;
    double axisAnchor4_low;
    double axisAnchor4_high;
    double axisAnchor5;

    CamPos s_CamPos;

    double cameraZ_vel_ang1;
    double cameraZ_vel_ang2;
    double cameraZ_vel_ang3;
    double cameraZ_vel_ang4;

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
    bool isStepRuning = false;
    bool isPtpmove = false;

    bool doorFlag = false;

    bool axisHomefinish = false;
    QMap<int,QPair<bool,bool>> pos_limitsFlag;//正负限位标志位
    AppConfig appconfig ;

    float startAxis0 =  9.5;
    float startAxis8 =  10.5;

    AxisFormula axisformula;

    QVector<AxisFormula::TableRow> vecTableRow;
    bool vecTableRowFlag = false;

public:
    bool vecStartFlag = false;

    float photoPos = 0;//探测器请求拍摄的位置
    float photoPosX = 0;//探测器请求拍摄的位置

    float m_endPos = 0;//探测器请求拍摄的位置
    float m_step  = 1;//探测器请求拍摄的位置

};

#endif // MOTIONCTRLCOMMAND_H
//错误码：3100 正向硬件限位触发，不允许正向启动vmove
//错误码：3101 负向硬件限位触发，不允许负向启动vmove
//错误码：1012 总线控制卡或控制器轴未使能，先使能轴再操作
//错误码: 1801  IO输入输出口超出最大允许值（16）或映射轴号超出范围
