#include "motioncommand.h"
#include <Windows.h>
#include <QtConcurrent/QtConcurrent>
#include "../third_party/device/include/LTDMC.h"
// #include "MainInterface/toastmsgwidget.h"
#include <iomanip>

#include <QMessageBox>
#include "motionconfig.h"

// #pragma execution_character_set("utf-8")

QMutex MotionCtrlCommand::m_Mutex{};

MotionCtrlCommand::MotionCtrlCommand(QObject *parent)
    : QObject{parent}
{
    mcInfoVec = QVector<MotionCtrlData::MotionCtrlInfo>(m_axisCount);

    adaptorIniPath = QCoreApplication::applicationDirPath() +"/adaptor.ini";
    QFileInfo adaptorFileInfo(adaptorIniPath);
    if(adaptorFileInfo.isFile())
    {
        adaptorSharedPointer.reset(new QSettings(adaptorIniPath,QSettings::IniFormat));
    }

    MotionConfig::readMotionConfig();

    QObject::connect(this,&MotionCtrlCommand::sig_moveAcqSpotComplete,this,&MotionCtrlCommand::slot_test);
}

MotionCtrlCommand::~MotionCtrlCommand()
{

    taskFuture.waitForFinished();
    if(readCntStsTimer) {

        readCntStsTimer->deleteLater();
        readCntStsTimer = nullptr;
    }
    if(readDataTimer) {
        readDataTimer->deleteLater();
        readDataTimer = nullptr;
    }

    if(readIODataTimer) {
        readIODataTimer->deleteLater();
        readIODataTimer = nullptr;
    }
    disconnect();

    MotionConfig::writeMotionConfig();
}

MotionCtrlCommand *MotionCtrlCommand::getInstance()
{
    QMutexLocker mutexLocker(&m_Mutex);
    static MotionCtrlCommand instance;
    return &instance;
}

bool MotionCtrlCommand::boardReset()
{
    if(isHaveTask) {
        qDebug() << "The current task not finished!";
        return false;
    }
    res = dmc_board_reset();
    if(res) {
        qDebug() << "dmc_board_reset Failed! Return ErrCode = " <<res;
        return false;
    }
    res = dmc_board_close();
    if(res) {
        qDebug() << "dmc_board_close Failed! Return ErrCode = " <<res;
        return false;
    }
    qDebug() <<"硬件复位开始----";
    taskFuture = QtConcurrent::run([=](){
        isHaveTask = true;
        for(int i = 15; i>=0; --i) {
            emit sig_resetTime(i);
            qDebug() <<"boardReset time = " <<i;
            QThread::msleep(1000);
        }
        res = dmc_board_init();
        if(res) {
            qDebug() << "dmc_board_init Failed! Return ErrCode = " <<res;
            return ;
        }
        qDebug() <<"硬件复位完成----";
        isHaveTask = false;
    });
    return true;
}

bool MotionCtrlCommand::softReset()
{
    if(isHaveTask) {
        qDebug() << "The current task not finished!";
        return false;
    }
    res = dmc_soft_reset(CurrCardNo);
    if(res) {
        qDebug() << "dmc_soft_reset Failed! Return ErrCode = " <<res;
        return false;
    } else {
        qDebug() <<"软件复位开始----";
        //TODO
        taskFuture = QtConcurrent::run([=](){
            isHaveTask = true;
            for(int i = 15; i>=0; --i) {
                emit sig_resetTime(i);
                qDebug() <<"softReset time = " <<i;
                QThread::msleep(1000);
            }
            qDebug() <<"软件复位完成----";

            isHaveTask = false;
        });
    }
    return true;
}

bool MotionCtrlCommand::ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode)
{
    if(axisNo == /*annulus_W*/4)
    {
        if(vel > 60 ) {
            vel = 60;
        }
        if(vel <= 0) {
            vel = 15;
        }
    }else
    {
        if(vel > 201 ) {
            vel = 200;
        }
        if(vel <= 0) {
            vel = 15;
        }
    }



    double startVel = 1, stopVel = 0;

    double Tacc = getValueFromAdaptorIni(QString("ParaAxis%1/TAcc").arg(axisNo)).toDouble();
    double Tdec = getValueFromAdaptorIni(QString("ParaAxis%1/TDec").arg(axisNo)).toDouble();

    qDebug()<<"axisNo "<<axisNo<<" startVel "<<startVel<<" vel "<<vel<<" Tacc "<<Tacc<<" Tdec "<<Tdec<<" stopVel "<<stopVel;
    res = dmc_set_profile_unit(CurrCardNo,axisNo,startVel,vel,Tacc,Tdec,stopVel);
    if(res) {
        qDebug() << "dmc_set_profile_unit Failed! Return ErrCode = " <<res;
        return false;
    }
    qDebug()<<"axisNo "<<axisNo<<" dist "<<dist<<" pos_mode "<<pos_mode;
    res = dmc_pmove_unit(CurrCardNo,axisNo,dist,pos_mode);
    if(res) {
        qDebug() << "dmc_vmove Failed! Return ErrCode = " <<res;
        return false;
    }
    return true;
}


bool MotionCtrlCommand::jogMove(ushort axisNo, double vel, ushort dir)
{
    if(axisNo != /*annulus_W*/4)
    {
        if(vel > 201 ) {
            vel = 200;
        }
        if(vel <= 0) {
            vel = 15;
        }
    }else
    {
        if(vel > 100 ) {
            vel = 25;
        }
        if(vel <= 0) {
            vel = 10;
        }
    }


    double startVel = 1, stopVel = 0;
    double  s_para = 0.0;
    double Tacc = getValueFromAdaptorIni(QString("ParaAxis%1/TAcc").arg(axisNo)).toDouble();
    double Tdec = getValueFromAdaptorIni(QString("ParaAxis%1/TDec").arg(axisNo)).toDouble();

    qDebug()<<"jogMove axisno:"<<axisNo<<" startvel:"<<startVel<<" vel:"<<vel<<" dir:"<<dir;
    res = dmc_set_profile_unit(CurrCardNo,axisNo,startVel,vel,Tacc,Tdec,stopVel);
    if(res) {
        qDebug() << "dmc_set_profile_unit Failed! Return ErrCode = " <<res;
        return false;
    }
    res = dmc_set_s_profile(CurrCardNo,axisNo,0,s_para);
    if(res) {
        qDebug() << "dmc_set_s_profile Failed! Return ErrCode = " <<res;
        return false;
    }
    res = dmc_vmove(CurrCardNo,axisNo,dir);
    if(res) {
        qDebug() << "dmc_vmove Failed! Return ErrCode = " <<res;
        return false;
    }
    return true;
}

bool MotionCtrlCommand::axHomingMove(ushort axisNo)
{
    if(isHaveTask) {
        qDebug() << "This task not finished!";
        return false;
    }

    ushort state = 0;
    res = nmc_get_axis_state_machine(CurrCardNo,axisNo,&state);
    if(state!=4)
    {
        qDebug()<<"Device not prepared, Curr state = "<<state;
    }
    if(res)
    {
        qDebug()<<"nmc_get_axis_state_machine Failed! Return ErrCode"<<res;
    }
    ushort hMode = 0;

    qDebug()<<"HomingMove axisNo:"<<axisNo;

    hMode = getValueFromAdaptorIni(QString("ParaAxis%1/HOMEMODE").arg(axisNo)).toInt();

    double lowVel = getValueFromAdaptorIni(QString("ParaAxis%1/HOMELOWSPEED").arg(axisNo)).toDouble();
    double highVel = getValueFromAdaptorIni(QString("ParaAxis%1/HOMEHIGHSPEED").arg(axisNo)).toDouble();

    double Tacc = getValueFromAdaptorIni(QString("ParaAxis%1/TAcc").arg(axisNo)).toDouble();
    double Tdec = getValueFromAdaptorIni(QString("ParaAxis%1/TDec").arg(axisNo)).toDouble();

    double offset = getValueFromAdaptorIni(QString("ParaAxis%1/HOMEPOS").arg(axisNo)).toDouble();

    qDebug()<<"home data:"<<hMode<<" lowVel "<<lowVel<<" highVel "<<highVel<<" offset "<<offset<<" axisNo "<<axisNo;
    // 设置回零参数
    res =  nmc_set_home_profile(CurrCardNo,axisNo,hMode,lowVel,highVel,Tacc,Tdec,offset);
    if(res) {
        qDebug() << "nmc_set_home_profile Failed! Return ErrCode = " <<res;
        return false;
    }
    //  回零
    res = nmc_home_move(CurrCardNo,axisNo);
    if(res) {
        qDebug() << "nmc_home_move Failed! Return ErrCode = " <<res;
        return false;
    }

    return true;
}

bool MotionCtrlCommand::axisHomingMoveFinish(ushort axisNo)
{
    ushort state;
    dmc_get_home_result(CurrCardNo,axisNo,&state);
    if(state != 1)
    {
        return false;
    }

    return true;
}

void MotionCtrlCommand::allaxisHoming()
{
    QtConcurrent::run([this](){
        QString msg = "";

        axHomingMove(Xray_X);
        axHomingMove(Xray_Z);

        axHomingMove(Material_X);
        axHomingMove(Material_Y);
        axHomingMove(Material_Z);

        axHomingMove(Detector_X);
        axHomingMove(Detector_Y);
        axHomingMove(Detector_Z);

        //== 1已停止   == 0 运动中
        while(dmc_check_done(CurrCardNo,Xray_X) == 0)
        {

        }
        //== 1已停止   == 0 运动中
        while(dmc_check_done(CurrCardNo,Xray_Z) == 0)
        {

        }

        while(dmc_check_done(CurrCardNo,Material_X) == 0)
        {

        }
        //== 1已停止   == 0 运动中
        while(dmc_check_done(CurrCardNo,Material_Y) == 0)
        {

        }

        while(dmc_check_done(CurrCardNo,Material_Z) == 0)
        {

        }
        //== 1已停止   == 0 运动中
        while(dmc_check_done(CurrCardNo,Detector_X) == 0)
        {

        }

        while(dmc_check_done(CurrCardNo,Detector_Y) == 0)
        {

        }

        while(dmc_check_done(CurrCardNo,Detector_Z) == 0)
        {

        }



        if(!axisHomingMoveFinish(Xray_X))
        {
            msg += QString(u8"射线源X轴 轴%1 回零失败 \n").arg(Xray_X);
        }
        msg += QString(u8"进料X轴 轴%1 回零完成 \n").arg(Xray_X);
        if(!axisHomingMoveFinish(Xray_Z))
        {
            msg += QString(u8"射线源Z轴 轴%1 回零失败 \n").arg(Xray_Z);
        }
        msg += QString(u8"射线源Z轴 轴%1 回零完成 \n").arg(Xray_Z);
        if(!axisHomingMoveFinish(Material_X))
        {
            msg += QString(u8"载物台X轴 轴%1 回零失败 \n").arg(Material_X);
        }
        msg += QString(u8"载物台X轴 轴%1 回零完成 \n").arg(Material_X);
        if(!axisHomingMoveFinish(Material_Y))
        {
            msg += QString(u8"载物台Y轴 轴%1 回零失败 \n").arg(Material_Y);
        }
        msg += QString(u8"载物台Y轴 轴%1 回零完成 \n").arg(Material_Y);
        if(!axisHomingMoveFinish(Material_Z))
        {
            msg += QString(u8"载物台Z轴 轴%1 回零失败 \n").arg(Material_Z);
        }
        msg += QString(u8"载物台Z轴 轴%1 回零完成 \n").arg(Material_Z);
        if(!axisHomingMoveFinish(Detector_X))
        {
            msg += QString(u8"探测器X轴 轴%1 回零失败 \n").arg(Detector_X);
        }
        msg += QString(u8"探测器X轴 轴%1 回零完成 \n").arg(Material_X);
        if(!axisHomingMoveFinish(Detector_Y))
        {
            msg += QString(u8"探测器Y轴 轴%1 回零失败 \n").arg(Detector_Y);
        }
        msg += QString(u8"探测器Y轴 轴%1 回零完成 \n").arg(Detector_Y);
        if(!axisHomingMoveFinish(Detector_Z))
        {
            msg += QString(u8"探测器Z轴 轴%1 回零失败 \n").arg(Detector_Z);
        }
        msg += QString(u8"探测器Z轴 轴%1 回零完成 \n").arg(Detector_Z);

        //判断是否回零完成加个弹窗
        emit sig_motionMsg(msg);
    });
}


bool MotionCtrlCommand::axEnable(ushort axisNo)
{
    QString msg;
    res =  nmc_set_axis_enable(CurrCardNo,axisNo);
    if(res) {
        // msg = QString(u8"轴 %1 使能失败 ! 错误码 = %2").arg(axisNo).arg(res);
        // qDebug() << msg;
        emit sig_motionMsg(u8"轴"+QString::number(axisNo)+"使能失败 ! 错误码 :"+QString::number(res));
        return false;
    }
    emit sig_motionMsg(u8"轴"+QString::number(axisNo)+"使能成功");
    // msg = QString("轴 %1 使能成功 !").arg(axisNo);
    return true;
}

bool MotionCtrlCommand::axDisenable(ushort axisNo)
{
    res =  nmc_set_axis_disable(CurrCardNo,axisNo);
    if(res) {
        qDebug() << "nmc_set_axis_disable Failed! Return ErrCode = " <<res;
        return false;
    }
    return true;
}

bool MotionCtrlCommand::axisStop(ushort axisNo, ushort stopMode)
{
    res =  dmc_stop(CurrCardNo,axisNo,stopMode);
    if(res) {
        qDebug() << "dmc_stop Failed! Return ErrCode = " <<res;
        return false;
    }
    return true;
}

bool MotionCtrlCommand::emgStop()
{
    res =  dmc_emg_stop(CurrCardNo);
    if(res) {
        qDebug() << "dmc_emg_stop Failed! Return ErrCode = " <<res;
        return false;
    }
    return true;
}

bool MotionCtrlCommand::readInbit(ushort bitNo, ushort *state)
{
    res =  dmc_read_inbit_ex(CurrCardNo,bitNo,state);
    if(res) {
        // qDebug() << "dmc_read_inbit_ex Failed! Return ErrCode = " <<res;
        return false;
    }
    return res;
}

bool MotionCtrlCommand::readOutbit(ushort bitNo, ushort *state)
{
    // qDebug()<<"dmc_read_outbit_ex("<<bitNo<<","<<*state<<")";
    res =  dmc_read_outbit_ex(CurrCardNo,bitNo,state);
    // qDebug()<<"MotionCtrlCommand::readOutbit :: state = " << *state;
    if(res) {
        qDebug() << "dmc_read_outbit_ex Failed! Return ErrCode = " <<res;
        return false;
    }
    return res;
}

bool MotionCtrlCommand::writeOutbit(ushort bitNo, ushort out)
{
    qDebug()<<"dmc_write_outbit("<<bitNo<<","<<out<<")";
    res =  dmc_write_outbit(CurrCardNo,bitNo,out);
    if(res) {
        qDebug() << "dmc_write_outbit Failed! Return ErrCode = " <<res;
        return false;
    }
    return res;
}

void MotionCtrlCommand::setDoStatus(ushort bitNo)
{
    ushort state = 0;
    readOutbit(bitNo,&state);

    writeOutbit(bitNo,!state);
}

bool MotionCtrlCommand::readParamInit(const QString &initFile)
{
    int res;
    qDebug()<<initFile.toLatin1();
    res = dmc_download_configfile(CurrCardNo,initFile.toLatin1());
    res = dmc_download_configfile_ex(CurrCardNo,initFile.toLatin1());
    if(res)
    {
        qDebug()<<"导入参数失败,返回值:"<<res;
        return false;
    }
    return true;
}
//EtherCAT i 总线轴轴号
void MotionCtrlCommand::nmcClearAxisErrcode()
{
    for(int i = 0;i<20;i++)
    {
        nmc_clear_axis_errcode(CurrCardNo,i);
    }
}

const int MotionCtrlCommand::getCardNo() const
{
    return CurrCardNo;
}


void MotionCtrlCommand::slot_startTimerTask()
{
    bool status = connect();
    if(!status) {
        qDebug() <<"板卡连接失败";
        return;
    }
    qDebug() <<"板卡连接成功";
    init();

    // writeOutbit(IoPortMap["lightGreen"],0);//设置三色灯绿
    // writeOutbit(IoPortMap["GreenLight"],0);//设置灯带绿

    // ushort total;
    // int res =  nmc_get_total_slaves(CurrCardNo,2,&total);//获取 EtherCAT 从站总数
    // qDebug()<<"total:"<<total;
    // if(res) {
    //     qDebug() << "nmc_get_total_slaves Failed! Return ErrCode = " <<res;
    // }

    qDebug() << "motionCtrl thread " << QThread::currentThreadId();
    readCntStsTimer = new QTimer();
    readDataTimer = new QTimer();
    readIODataTimer = new QTimer();
    QObject::connect(this->readCntStsTimer,&QTimer::timeout,this,&MotionCtrlCommand::readCntStatus);
    QObject::connect(this->readDataTimer,&QTimer::timeout,this,&MotionCtrlCommand::readData);
    QObject::connect(this->readIODataTimer,&QTimer::timeout,this,&MotionCtrlCommand::readIOStatus);
    readCntStsTimer->start(300);
    readDataTimer->start(200);
    readIODataTimer->start(500);

}

void MotionCtrlCommand::slot_stopTimerTask()
{
    if(stopFlag)
    {
        qDebug() << "stop this thread " << QThread::currentThreadId();
        if(readCntStsTimer && readCntStsTimer->isActive()) {
            readCntStsTimer->stop();
        }
        if(readDataTimer && readDataTimer->isActive()) {
            readDataTimer->stop();
        }
        if(readIODataTimer && readIODataTimer->isActive())
        {
            readIODataTimer->stop();
        }

        isRun = false;
        stopFlag = false;
    }


}

void MotionCtrlCommand::slot_test()
{
    qDebug()<<"MotionCtrlCommand::slot_test";
}

void MotionCtrlCommand::slot_releaseIsRun()
{
    qDebug()<<"slot_releaseIsRun isRun == false";
}

void MotionCtrlCommand::updateMotionParams(const QString& configPath)
{
    if(!configSharedPointer.isNull())
    {
        configSharedPointer.reset(new QSettings(iniPath,QSettings::IniFormat));
    }
}

//读取 EtherCAT 总线状态
//PortNum EtherCAT 总线端口号，固定为 2
//errcode EtherCAT 总线状态，0 表示正常
void MotionCtrlCommand::readCntStatus()
{
    ushort errCode;
    short ret = nmc_get_errcode(CurrCardNo, 2, &errCode);
    if(ret) {
        return;
    }

    if(errCode == 0) {
        isConnected = true;
        emit sig_connect(true);
    } else {
        isConnected = false;
        emit sig_connect(false);
    }

}

void MotionCtrlCommand::readData()
{
    if(!isConnected) {
        return;
    }
    MachineStatus ms =MS_NONE;
    bool axisHomefinish = false;
    for (int i = 0; i < mcInfoVec.size(); ++i) {
        ushort axisNo = i;
        ushort enable;
        int res = nmc_get_axis_state_machine(CurrCardNo,axisNo,&enable);
        // qDebug()<<"readData res:"<<res;

        enable == 4?mcInfoVec[i].enable = true : mcInfoVec[i].enable = false;
        ulong axSts = dmc_axis_io_status(CurrCardNo,axisNo);
        // qDebug()<<"dmc_axis_io_status "<< i;
        (axSts)&0x01?mcInfoVec[i].almWarn = true : mcInfoVec[i].almWarn = false;//伺服报警信号
        (axSts>>1)&0x01?mcInfoVec[i].hardPosLmtWarn = true : mcInfoVec[i].hardPosLmtWarn = false;//正限位
        (axSts>>2)&0x01?mcInfoVec[i].hardNegLmtWarn = true : mcInfoVec[i].hardNegLmtWarn = false;//负限位
        (axSts>>3)&0x01?mcInfoVec[i].emgStop = true : mcInfoVec[i].emgStop = false;//急停
        (axSts>>4)&0x01?mcInfoVec[i].org = true : mcInfoVec[i].org = false;//原点
        (axSts>>6)&0x01?mcInfoVec[i].softPosLmtWarn = true : mcInfoVec[i].softPosLmtWarn = false;//正软限位
        (axSts>>7)&0x01?mcInfoVec[i].softNegLmtWarn = true : mcInfoVec[i].softNegLmtWarn = false;//负软限位

        if(mcInfoVec[i].almWarn)
        {
            ms = MS_WARN;
        }else if(mcInfoVec[i].hardPosLmtWarn || mcInfoVec[i].hardNegLmtWarn || mcInfoVec[i].emgStop || mcInfoVec[i].softPosLmtWarn || mcInfoVec[i].softNegLmtWarn)
        {
            ms = MS_ALARM;
        }

        short busy = dmc_check_done(CurrCardNo,axisNo);//运动完成
        // qDebug()<<"dmc_check_done "<<i;
        busy?mcInfoVec[i].busy = false : mcInfoVec[i].busy = true;

        double current_speed = 0;//速度
        dmc_read_current_speed_unit(CurrCardNo,axisNo,&current_speed);
        mcInfoVec[i].axEncVel = current_speed;
        double current_pos = 0;
        if(current_pos < 0.000001)
            current_pos = 0.0;

        dmc_get_position_unit(CurrCardNo,axisNo,&current_pos);//读取位置

        mcInfoVec[i].axEncPos = current_pos;

        axisHomefinish =  axisHomingMoveFinish(axisNo);

        mcInfoVec[i].ZeroingIsComplete =  axisHomefinish;
        // qDebug()<<"mcInfoVec "<<axisNo<<" :"<<current_pos<<" pos_limits[axisNo].first :"<<pos_limits[axisNo].first <<" pos_limits[axisNo].second: "<<pos_limits[axisNo].second;
    }

    QVariant var;
    var.setValue(mcInfoVec);
    emit sig_dataChanged(var);

}

void MotionCtrlCommand::readIOStatus()
{
    ushort yFMove;

    DIStatus distatus;
    QVector<DIStatus> DIPortVec;//输入IO

    for(int i = 0 ;i<150;i++)
    {

        readInbit(i,&yFMove);
        distatus.IO_Port = i;
        // qDebug()<<"IO status:"<<i<<" status:"<<yFMove;
        distatus.IO_Status = yFMove;
        DIPortVec.push_back(distatus);

    }
    QVariant var;
    var.setValue(DIPortVec);

    emit sig_DIStatusdataChanged(var);

}

bool MotionCtrlCommand::connect()
{

    short num = dmc_board_init();

    if(num <= 0 || num>=20) {
        qDebug() <<"Don't have motionCard!";
        // ToastMsgWidget *toast// msg = new ToastMsgWidget();
        // toastMsg->showMessage(u8"未检测到控制板卡,请检查原因");
        qDebug()<<u8"未检测到控制板卡,请检查原因";
        return false;
    }
    // TODO: Add extra initialization here
    WORD cardNum;      //定义卡数
    WORD cardList[8];   //定义卡号数组
    DWORD cardTypeList[8];   //定义各卡类型
    res = dmc_get_CardInfList(&cardNum,cardTypeList,cardList);
    if(res) {
        qDebug() << "dmc_get_CardInfList Failed! Return ErrCode = " <<res;
        return false;
    }

    CurrCardNo = cardList[0];

    return true;
}

bool MotionCtrlCommand::disconnect()
{
    res =  dmc_board_close();
    if(res) {
        qDebug() << "dmc_board_close Failed! Return ErrCode = " <<res;
        return false;
    }
    return true;
}

bool MotionCtrlCommand::init()
{
    //使能所有轴
    // bool status = axEnable(255);
    for(int i=0;i < m_axisCount;i++)
    {
        bool status = axEnable(i);
        if(!status) {
            qDebug() <<" axis:"<<i<<" 板卡使能失败";
            // return false;
        }else
        {
            qDebug() <<" axis:"<<i<<" 板卡使能成功";
        }
    }
    // bool status = axEnable(255);
    // if(!status) {
    //     qDebug() <<"板卡使能失败";
    //     return false;
    // }
    // qDebug() <<"板卡使能成功";

    // nmc_set_offset_pos(getCardNo(),axisNo7,0);
    // nmc_set_offset_pos(getCardNo(),axisNo1,0);
    // nmc_set_offset_pos(getCardNo(),axisNo2,0);

    readParamInit("adaptor.ini");
//    allAxisResetHoming();
//    moveAcqSpot();


    return true;
}

//扫描完成
void MotionCtrlCommand::slot_beamContiShootFinished()
{
    tasktype = TASK_NONE;
    qDebug()<<"1111MotionCtrlCommand::slot_beamContiShootFinished() tasktype = TASK_NONE";
    axisStop(/*annulus_W*/4,0);//dd马达紧急停止
    qDebug()<<" MotionCtrlCommand::slot_beamContiShootFinished tasktype = TASK_NONE";
    // this->setBDdEms(true);
    // QTimer::singleShot(200,this,[=]{
    //     this->setBDdEms(false);
    // });
}

void MotionCtrlCommand::slot_beamStepShootFinished()
{
    this->isShootFinished = true;
    qDebug() << "isShootFinished = " <<  this->isShootFinished;
}

//连续扫描
void MotionCtrlCommand::beamContinuousScanning(double degSpeed,double start,double angleRange,float step)
{
    qDebug()<<u8"旋转载物台 旋转到位 m_wPosFinish"<<m_wPosFinish;
    //旋转载物台 旋转到位
    if(!m_wPosFinish)
    {
        qDebug()<<u8"旋转载物台 旋转到位 m_wPosFinish 停止";
        return;
    }
    qDebug()<<" beamContinuousScanning m_wPosFinish:"<<m_wPosFinish;
    m_wPosFinish = false;

    if(tasktype != TASK_NONE) {
        qDebug() << "task is running!";
        return;
    }
    tasktype = TASK_CONTINUINGMOVE;
    continueMoveSpeed = degSpeed;

    taskFuture = QtConcurrent::run([=]{
        //== 1已停止   == 0 运动中
        while(dmc_check_done(CurrCardNo,/*annulus_W*/4) == 0)
        {
            QThread::sleep(0.3);
        }
        while(isRun) {
            try {
                readDDActPos(start,angleRange,step);//读取DD马达旋转轴位置 、回零、开始运动360*10位置
            } catch (std::exception& e) {
                qDebug() << "Exception: " <<e.what();
            }
            //            qDebug()<<__FUNCTION__<<" tasktype:"<<tasktype;
            if(tasktype == TASK_NONE) {
                double current_pos = 0;
                if(current_pos < 0.000001)
                    current_pos = 0.0;
                dmc_get_position_unit(CurrCardNo,/*annulus_W*/4,&current_pos);//dd马达旋转轴
                qDebug() <<"End ddActPos "<< current_pos;
                qint64 endTime = QDateTime::currentMSecsSinceEpoch();

                unsigned short hcmp_0 = 0;          //比较器号
                long remained_points = 0;          //返回可添加比较点数
                double current_point = 0;          //返回当前比较点位置
                long ruuned_points = 0;          //返回已比较点位

                dmc_hcmp_get_current_state_unit(CurrCardNo,hcmp_0,&remained_points,&current_point,&ruuned_points);
                qDebug()<<u8"圆环 扫描结束 返回已比较点位:"<<ruuned_points;

                qDebug() <<"End Time "<<endTime;
                qDebug() <<"move Time" <<endTime - startTime <<"ms";
                break;
            }
        }
        qDebug() << "beamContinuousScanning finished";
    });
}


void MotionCtrlCommand::beamStepScanning(int degRange, int step)
{
    if(taskFuture.isRunning()) {
        return;
    }
    taskFuture = QtConcurrent::run([=]()->void{
        isStepRun = true;
        int i = 0;
        for(; i <= degRange ;i+=step ) {
            qDebug() <<"ddAbsMove = " <<i;
            if(i == 0) {
                //                // 复位
                // this->setBDdReset(true);
                // // 脉冲信号复位
                // this->setBDdReset(false);
                // // 回零
                // this->setBDdHomeStart(true);
                // // 脉冲信号复位
                // this->setBDdHomeStart(false);
                axHomingMove(/*annulus_W*/4);

                QThread::msleep(3000);
            } else {
                // this->ddAbsMove(step,50,true);
                this->ptpMove(/*annulus_W*/4,0.5,step,1);
                // QThread::msleep(100);
                // this->ddAbsStop();
            }
            while(isRun && isEmsStop == false) {
                bool val = dmc_check_done(CurrCardNo,/*annulus_W*/4);
                if((!val)|| (i == 0 && val)) {
                    qDebug() << "TFCommandData::sig_beamStepScanningStart";
                    emit sig_beamStepScanningStart();
                    break;
                }
            }
            while(isRun && isEmsStop == false) {
                bool val = dmc_check_done(CurrCardNo,/*annulus_W*/4);
                if(val){
                    qDebug() << "TFCommandData::sig_beamStepScanningEnd";
                    emit sig_beamStepScanningEnd();
                    break;
                }
            }
            //拍图--------
            //等待拍图完成-------
            while(isRun  && isEmsStop == false) {
                if(isShootFinished) {
                    qDebug() <<"拍图完成!-------";
                    break;
                }
            }
            isShootFinished = false;
        }
        if((i - step)>degRange && isEmsStop == false) {
            int currDeg = degRange - i + step;
            // this->ddAbsMove(currDeg,50,true);
            this->ptpMove(/*annulus_W*/4,0.5,currDeg,1);
            // this->ddAbsStop();
            bool isStart = false;
            while(isRun && isEmsStop == false) {
                bool val =  dmc_check_done(CurrCardNo,/*annulus_W*/4);
                if((!val)|| (i == 0 && val)) {
                    isStart = true;
                    qDebug() << "TFCommandData::sig_beamStepScanningStart";
                    emit sig_beamStepScanningStart();
                }
                if((val&&isStart)|| (i == 0 && val)){
                    qDebug() << "TFCommandData::sig_beamStepScanningEnd";
                    emit sig_beamStepScanningEnd();
                    break;
                }
            }
            //拍图--------
            //等待拍图完成-------
            while(isRun && isEmsStop == false) {
                if(isShootFinished) {
                    break;
                }
            }
            isShootFinished = false;
        }
        if(isEmsStop) {
            isEmsStop = false;

        }
        isStepRun = false;
        qDebug() << "Finished";
    });
}

short MotionCtrlCommand::lineHcmp(int increment)
{

    unsigned short hcmp_0 = 0;          //比较器号
    unsigned short axis = 0;            //辅助编码器
    unsigned short cmp_mode_0 = 5;      // 比较模式，5线性
    unsigned short cmp_source=1;          //比较位置源，固定值 1：辅助编码器计数器
    unsigned short cmp_logic=0;          //有效电平： 0：低电平， 1：高电平
    int time = 1000;       //脉冲宽度，单位： us，取值范围： 1us~20s
    int cmp_pos= 0;          //添加比较位置
    // int increment = 100;  //设置高速比较线性模式参数，位置增量值
    int count = 50000;          //设置高速比较线性模式参数，比较次数

    qDebug()<<"lineHcmp "<<increment;

    //位置清零
    // dmc_set_position_unit(CurrCardNo, 0, 0);
    unsigned short hcmp = 0;
    for(int i = 0;i<10;i++)
    {
        dmc_hcmp_clear_points(CurrCardNo, 0);//清除比较点

        dmc_set_extra_encoder(CurrCardNo,axis,0);//清除编码器位置
    }


    dmc_hcmp_set_mode(CurrCardNo, hcmp_0, cmp_mode_0);//配置比较器0  比较模式

    dmc_hcmp_set_config(CurrCardNo, hcmp_0, axis, cmp_source, cmp_logic, time);//配置比较器0关联轴0，比较源为辅助编码器计数器

    dmc_hcmp_clear_points(CurrCardNo, 0);//清除比较点

    short linecmp = dmc_hcmp_set_liner_unit(CurrCardNo, hcmp_0, increment, count);//配置比较器0  设置线性比较参数

    dmc_hcmp_add_point_unit(CurrCardNo, hcmp_0, cmp_pos);//配置比较器0  添加比较点位置

    emit sig_motionMsg(u8"板卡打开高速一维比较功能 参数 步长:"+QString::number(increment));

    return linecmp;
}


void MotionCtrlCommand::readDDActPos(double start,double angleRange,float step)
{
    double current_pos = 0;
    int range;


    dmc_get_position_unit(CurrCardNo,/*annulus_W*/4,&current_pos);//dd马达旋转轴
    if(current_pos < 0.0001)
        current_pos = 0.0;

    double startPos = 0;
    if(start > angleRange)
    {
        startPos = start + MotionConfig::averageSpeedDist + MotionConfig::averageSpeedDist;
    }else
    {
        startPos = start;
    }


    // 判断是否需要进行回零
    if(tasktype == TASK_CONTINUINGMOVE)
    {
        // qDebug()<<u8"readDDActPos 起始位置 start: "<<start <<u8" 终止角:"<<angleRange<<" step:"<<step <<u8" 均速距离:"<<MotionConfig::averageSpeedDist
        //          <<" startPos："<<startPos<<" current_pos"<<int(current_pos);
        if(qFuzzyCompare(current_pos,startPos))//比较是否到起始位置度
        {
            // qDebug()<<"isRun :"<<isRun;

            tasktype = TASK_CONTINUINGMOVE_START;
            qDebug() << "TASK_CONTINUINGMOVE_START";
        }
        else
        {
            tasktype = TASK_CONTINUINGMOVE_HOME;
            resetMove = false;
            qDebug() << "TASK_CONTINUINGMOVE_HOME ";
        }
        // emit sig_beamContiScanningStart();
    }
    //回零任务
    if(tasktype == TASK_CONTINUINGMOVE_HOME) {
        // qDebug()<<u8"readDDActPos 起始位置 start: "<<start <<u8" 终止角:"<<angleRange<<" step:"<<step <<u8" 均速距离:"<<MotionConfig::averageSpeedDist
        //          <<" startPos："<<startPos<<" current_pos"<<float(current_pos)<<dmc_check_done(CurrCardNo,annulus_W);
        // resetMove = false;
        if(qFuzzyCompare(current_pos,startPos))//如果到了470
        {
            qDebug()<<u8"圆环到达起始位置："<<startPos;
            QThread::usleep(10);
            axisStop(/*annulus_W*/4,1);
            QThread::msleep(10);
            // while(dmc_check_done(CurrCardNo,annulus_W) == 1)//停止运动
            // {

            //     tasktype = TASK_CONTINUINGMOVE_START;
            //     qDebug() << "TASK_CONTINUINGMOVE_START";

            //     break;
            // }

        }
        else
        {
            // qDebug() << "homeMove resetMove："<<resetMove;
            if(resetMove) {
                // // 0：指定轴正在运行，1：指定轴已停止
                // while(dmc_check_done(CurrCardNo,annulus_W) == 0)//正在运动
                // {
                //     QThread::msleep(3);
                // }
                // qDebug()<<"圆环停止运动 到达位置 ："<<current_pos;
                // float comNum = qAbs(current_pos - startPos);
                // if(comNum < 0.0001)
                // {
                //     while(dmc_check_done(CurrCardNo,annulus_W) == 1)
                //     {
                //         tasktype = TASK_CONTINUINGMOVE_START;
                //         qDebug() << "homeMove -> TASK_CONTINUINGMOVE_START";
                //         break;
                //     }

                // }else
                // {
                //     double vel = MotionConfig::lastAxisSpeed[annulus_W];
                //     ptpMove(annulus_W,vel,startPos,1);
                // }
                return;
            }

            qDebug() << u8"滑环开始运动 目标点位 ：起始位置 ";
            axisStop(/*annulus_W*/4,1);
            QThread::msleep(10);
            resetMove = true;
            // axisDDToZero(startPos);//如果没有到470，就跑到470
            double vel = MotionConfig::lastAxisSpeed[/*annulus_W*/4];
            ptpMove(/*annulus_W*/4,vel,startPos,1);
        }
    }
    //开始移动
    if(tasktype == TASK_CONTINUINGMOVE_START)
    {
        tasktype = TASK_CONTINUINGMOVE_SHOOT;
        if(qFuzzyCompare(current_pos,startPos))//如果到了470
        {
            int hcmpNum = step/0.0001;//10000一度
            if(start > angleRange)
            {
                hcmpNum = hcmpNum *(-1);
            }else
            {
                hcmpNum = hcmpNum;
            }
            qDebug()<<"beamContinuousScanning hcmpNum :"<<hcmpNum;
            short linecmp =  lineHcmp(hcmpNum);//高速一维比较

            if(linecmp != 0)
                return ;


            QTimer::singleShot(10,this,[angleRange,start,startPos,&range,this]{

                qDebug() << "TASK_CONTINUINGMOVE_RUNING------";
                // int range = 470 - angleRange-90/*00*/;//从470跑到0，如果范围给380，那么380+90 ，470-（380+90）

                if(start>angleRange)
                {
                    range = angleRange ;//从470跑到0，如果范围给380，那么380+90 ，470-（380+90）
                }else
                {
                    range = angleRange+MotionConfig::averageSpeedDist +MotionConfig::averageSpeedDist /*00*/;//从470跑到0，如果范围给380，那么380+90 ，470-（380+90）
                }
                // qDebug()<<"range:"<<range;
                emit sig_motionMsg(u8"开始扫描 旋转轴 起始点 "+ QString::number(startPos)+u8" 均速距离："+ QString::number(MotionConfig::averageSpeedDist)
                                   + u8"终止位置："+ QString::number(range));
                ptpMove(/*annulus_W*/4,continueMoveSpeed,range,1);
            });
        }



    }
    //滑环不需要
    //移动至拍照位
    if(tasktype == TASK_CONTINUINGMOVE_SHOOT) {

        // qDebug() <<__FUNCTION__<< u8"起始位置："<< start<< u8" 终止位置："<< angleRange;

        float pos = 0;
        if(start > angleRange)//起始位置大于终止位置
        {
            pos = start + MotionConfig::averageSpeedDist;
            if(current_pos <= pos)//180-10的地方取图
            {
                startTime = QDateTime::currentMSecsSinceEpoch();
                qDebug() <<__FUNCTION__<< "start ddActPos  "<< current_pos;
                qDebug() <<__FUNCTION__<< "start Time  "<< startTime;
                tasktype = TASK_CONTINUINGMOVE_RUNING;
                qDebug() << u8"反转 TASK_CONTINUINGMOVE_RUNING 触发拍照";
                emit sig_allowBeamShooting();//触发拍照
                emit sig_motionMsg(u8"板卡 旋转轴 到达"+QString::number(current_pos)+" 触发采图");

                //            qDebug() << "sig_allowBeamShooting";
            }
        }else
        {
            pos = start + MotionConfig::averageSpeedDist;
            if(current_pos >= pos)//470-90的地方取图
            {
                startTime = QDateTime::currentMSecsSinceEpoch();
                qDebug() <<__FUNCTION__<< "start ddActPos  "<< current_pos;
                qDebug() <<__FUNCTION__<< "start Time  "<< startTime;
                tasktype = TASK_CONTINUINGMOVE_RUNING;
                qDebug() << u8"正转 TASK_CONTINUINGMOVE_RUNING 触发拍照";
                emit sig_allowBeamShooting();//触发拍照   软触发
                emit sig_motionMsg(u8"板卡 旋转轴 到达"+QString::number(current_pos)+" 触发采图");

            }
        }

    }
    if(tasktype == TASK_CONTINUINGMOVE_RUNING)
    {
        float pos = 0;
        if(start > angleRange)//起始位置大于终止位置
        {
            pos = angleRange + MotionConfig::averageSpeedDist;
            if(current_pos <= pos)//180-10的地方取图
            {
                startTime = QDateTime::currentMSecsSinceEpoch();
                qDebug() <<__FUNCTION__<< "TASK_CONTINUINGMOVE_RUNING End ddActPos  "<< current_pos;
                qDebug() <<__FUNCTION__<< "TASK_CONTINUINGMOVE_RUNING End Time  "<< startTime;

                qDebug() << u8"反转  运动结束！！！！！！";
                emit sig_TheSlipRingMovemenIsCompleted();//反转  运动结束
                // emit sig_motionMsg(u8"板卡 旋转轴 到达"+QString::number(current_pos)+" 触发采图");

                //            qDebug() << "sig_allowBeamShooting";
            }
        }else
        {
            pos = angleRange - MotionConfig::averageSpeedDist;
            if(current_pos >= pos)//470-90的地方取图
            {
                startTime = QDateTime::currentMSecsSinceEpoch();
                qDebug() <<__FUNCTION__<< "TASK_CONTINUINGMOVE_RUNING End ddActPos  "<< current_pos;
                qDebug() <<__FUNCTION__<< "TASK_CONTINUINGMOVE_RUNING End Time  "<< startTime;
                // tasktype = TASK_CONTINUINGMOVE_RUNING;
                qDebug() << u8"正转 运动结束！！！！！！";
                emit sig_TheSlipRingMovemenIsCompleted();//正转 运动结束
                // emit sig_motionMsg(u8"板卡 旋转轴 到达"+QString::number(current_pos)+" 触发采图");

            }
        }
    }

}

void MotionCtrlCommand::slot_beamEmsStop()
{
    qDebug()<<"333MotionCtrlCommand::slot_beamEmsStop() tasktype = TASK_NONE";
    tasktype = TASK_NONE;
    isEmsStop = true;
    if(!isStepRun) {
        isEmsStop = false;
    }
    this->axisStop(/*annulus_W*/4,1);
}
double MotionCtrlCommand::getAnnulusWPos()
{
    double current_pos;
    dmc_get_position_unit(CurrCardNo,/*annulus_W*/4,&current_pos);//读取位置
    return current_pos;
}

float MotionCtrlCommand::actLinearCompensationOfmotor(std::vector<std::vector<float> > &locationAndErr, float currLocation)
{
    int numPosition = locationAndErr.size();


    //边界检测。判断currLocation是否在locationAndErr中包含的范围中。
    if (currLocation < locationAndErr[0][0] || currLocation > locationAndErr[numPosition - 1][0])
    {
        qDebug()<<"currLocation: "<<currLocation<<" locationAndErr[0][0]";
        return currLocation; //如果超过范围，输出原值。
    }
    else
    {

        for (int i = 0; i < numPosition - 1; i++)
        {
            if (currLocation <= locationAndErr[i + 1][0])
            {

                //y = (y2 - y1)/(x2 - x1)*(x - x1) + y1;
                float err = (locationAndErr[i + 1][1] - locationAndErr[i][1]) / (locationAndErr[i + 1][0] - locationAndErr[i][0]) * (currLocation - locationAndErr[i][0]) + locationAndErr[i][1];
//                qDebug()<<"axis 4 :"<<currLocation - err;
                return currLocation - err;

            }
        }

    }
}

//三圈扫描完成 让滑环回到第一圈起始位
void MotionCtrlCommand::SlipRingBackToStartingPos(double startPos)
{
    qDebug()<<"SlipRingBackToStartingPos :"<<startPos;
    axisStop(/*annulus_W*/4,1);
    QThread::msleep(50);
    double vel = MotionConfig::lastAxisSpeed[/*annulus_W*/4];

    ptpMove(/*annulus_W*/4,vel,startPos,1);
}


QString MotionCtrlCommand::getValueFromAdaptorIni(QString str)
{
    if(adaptorSharedPointer.isNull()) {
        return "";
    }
    return adaptorSharedPointer->value(str).toString();
}

// void MotionCtrlCommand::setValue2AdaptorIni(QString str, QString value)
// {
//     if(adaptorSharedPointer.isNull()) {
//         return;
//     }
//     adaptorSharedPointer->setValue(str,value);
// }
