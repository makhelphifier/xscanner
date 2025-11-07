#include "motionctrlcommand.h"
#include <Windows.h>
#include <QtConcurrent>
#include "LTDMC.h"
#include "util/logger/logger.h"
QMutex MotionCtrlCommand::m_Mutex{};

MotionCtrlCommand::MotionCtrlCommand(QObject *parent)
    : QObject{parent}
{
    mcInfoVec = QVector<MotionCtrlData::MotionCtrlInfo>(9);
    iniPath = QCoreApplication::applicationDirPath() +"/MotionCtrlConfig.ini";
    QFileInfo fileInfo(iniPath);
    if(fileInfo.isFile())
    {
        configSharedPointer.reset(new QSettings(iniPath,QSettings::IniFormat));
    }

    adaptorIniPath = QCoreApplication::applicationDirPath() +"/adaptor.ini";
    QFileInfo adaptorFileInfo(adaptorIniPath);
    if(adaptorFileInfo.isFile())
    {
        adaptorSharedPointer.reset(new QSettings(adaptorIniPath,QSettings::IniFormat));
    }


    setParams();
    //b_scanLoop = false;
    IoPortMap.insert("openDoor",10);
    // IoPortMap.insert("closeDoor",9);

    // IoPortMap.insert("stop",11);
    // IoPortMap.insert("reset",12);
    // IoPortMap.insert("backLockDoor",13);
    // IoPortMap.insert("leftLockDoor",14);

    IoPortMap.insert("YellowLight",18);
    IoPortMap.insert("GreenLight",19);
    IoPortMap.insert("RedLight",20);
    IoPortMap.insert("Alarm",21);

    //OpenDoorMotorFrwardRotation 开门电机正转
    //OpenDoorMotorInversion 开门电机反转
    //MultipleSpeed1 多段速1
    //MultipleSpeed2 多段速2
    // IoPortMap.insert("OpenDoorMotorFrwardRotation",40);
    // IoPortMap.insert("OpenDoorMotorInversion",41);
    // IoPortMap.insert("MultipleSpeed1",42);
    // IoPortMap.insert("MultipleSpeed2",43);


    // IoPortMap.insert("lightYellow",44);
    // IoPortMap.insert("lightRed",45);
    // IoPortMap.insert("lightGreen",46);

    // IoPortMap.insert("safetyRelayReset",47);





    QObject::connect(this,&MotionCtrlCommand::sig_moveAcqSpotComplete,this,&MotionCtrlCommand::slot_test);
}

MotionCtrlCommand::~MotionCtrlCommand()
{
    qDebug()<<"~MotionCtrlCommand()";
    isRun = false;
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
    // HcmpSetMode();
    // axDisenable(255);
    // disconnect();
    // writeOutbit(IoPortMap["lightGreen"],1);//设置三色灯绿
    // writeOutbit(IoPortMap["GreenLight"],1);//设置灯带绿

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

std::vector<std::vector<float>> MotionCtrlCommand::csvParsing(QString filename)
{
    std::vector<std::vector<float>> locationAndErr;
    // 打开 CSV 文件
    // QFile file("D:/study/Project/csvparsing/csvParsing/data1.csv");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    }
    // qDebug() << "打开文件";

    QTextStream in(&file);

    // 正则表达式匹配 CSV 行（支持带引号的字段）
    QRegularExpression regex("(\"[^\"]*\"|[^,]+)");

    int row = 0;
    // 逐行读取文件
    while (!in.atEnd()) {
        QString line = in.readLine(); // 读取一行
        // qDebug()<<"line:"<<line;
        QRegularExpressionMatchIterator matches = regex.globalMatch(line);

        QStringList fields;
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString field = match.captured(0).trimmed();
            if (field.startsWith('"') && field.endsWith('"')) {
                field = field.mid(1, field.length() - 2); // 去除引号
            }
            fields.append(field);
        }

        // 确保至少有两列数据
        if (fields.size() >= 2) {
            QString column1 = fields.at(0); // 第一列数据
            QString column2 = fields.at(1); // 第二列数据

            locationAndErr[row][0] =column1.toFloat() ;
            locationAndErr[row][1] =column1.toFloat() ;

            qDebug() << "Column 1:" << column1 << "Column 2:" << column2;
        }
        row++;
    }
    qDebug()<<"row :"<<row;
    // 关闭文件
    file.close();

    return locationAndErr;
}



bool MotionCtrlCommand::ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode)
{
    if(axisNo == 4)
    {
        if(vel > 150 ) {
            vel = 150;
        }
        if(vel <0) {
            vel = 0;
        }
    }else
    {
        if(vel > 50 ) {
            vel = 50;
        }
        if(vel <0) {
            vel = 0;
        }
    }


    if(axisNo == AXIS::Detector_W)
    {
        Sleep(100);
    }

    double startVel = 1, stopVel = 0;
    double Tacc = 0.1, Tdec = 0.1;
    Tacc = getValueFromAdaptorIni(QString("ParaAxis%1/TAcc").arg(axisNo)).toDouble();
    Tdec = getValueFromAdaptorIni(QString("ParaAxis%1/TDec").arg(axisNo)).toDouble();

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
    if(axisNo == 4)
    {
        if(vel > 150 ) {
            vel = 150;
        }
        if(vel <0) {
            vel = 0;
        }
    }else
    {
        if(vel > 50 ) {
            vel = 50;
        }
        if(vel <0) {
            vel = 0;
        }
    }

    double startVel = 1, stopVel = 0;
    double Tacc = 0.1, Tdec = 0.1, s_para = 0.0;
    Tacc = getValueFromAdaptorIni(QString("ParaAxis%1/TAcc").arg(axisNo)).toDouble();
    Tdec = getValueFromAdaptorIni(QString("ParaAxis%1/TDec").arg(axisNo)).toDouble();

    // qDebug()<<"jogMove axisno:"<<axisNo<<" startvel:"<<startVel<<" vel:"<<vel<<" dir:"<<dir;
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


    double lowVel = getValueFromAdaptorIni(QString("ParaAxis%1/HOMELOWSPEED").arg(axisNo+1)).toDouble();
    double highVel = getValueFromAdaptorIni(QString("ParaAxis%1/HOMEHIGHSPEED").arg(axisNo+1)).toDouble();
    double Tacc = getValueFromAdaptorIni(QString("ParaAxis%1/HOMEACC").arg(axisNo)).toDouble();
    double Tdec = getValueFromAdaptorIni(QString("ParaAxis%1/HOMEDEC").arg(axisNo)).toDouble();

    double offset = getValueFromIni("HomingPara/offset").toDouble();

    qDebug()<<"home data:"<<hMode<<" lowVel "<<lowVel<<" Tdec "<<Tdec<<" Tacc "<<Tacc<<" axisNo "<<axisNo;
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
        // qDebug()<<"axisNo."<<axisNo<<" Homing failed Err Code = "<<state;
        return false;
    }
    return true;
}

void MotionCtrlCommand::setPosLimit(QMap<int, QPair<bool, bool> > pos_limitsFlagmap)
{
    pos_limitsFlag = pos_limitsFlagmap;
}

QMap<int, QPair<bool, bool> > MotionCtrlCommand::getPosLimit()
{
    return pos_limitsFlag;
}

bool MotionCtrlCommand::axEnable(ushort axisNo)
{
    res =  nmc_set_axis_enable(CurrCardNo,axisNo);
    if(res) {
        qDebug() << "nmc_set_axis_enable Failed! Return ErrCode = " <<res;
        return false;
    }
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
//     qDebug()<<"dmc_write_outbit("<<bitNo<<","<<out<<")";
    res =  dmc_write_outbit(CurrCardNo,bitNo,out);
    if(res) {
        qDebug() << "dmc_write_outbit Failed! Return ErrCode = " <<res;
        return false;
    }
    return res;
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

void MotionCtrlCommand::HcmpSetMode(unsigned short cmp_logic)
{
    // TODO: Add your control notification handler code here
    unsigned short hcmp_0 = 0;          //比较器号 对应硬件 CMP 端口
    unsigned short axis = 4;            //关联轴号 旋转轴
    unsigned short cmp_mode_0 = 4;      // 比较模式，4 队列
    unsigned short cmp_source=1;          //比较位置源，固定值 1：辅助编码器计数器
    // unsigned short cmp_logic=1;          //有效电平： 0：低电平， 1：高电平
    int time = 100000;					  //脉冲宽度，单位： us，取值范围： 1us~20s
    int cmp_pos=2;					  //添加比较位置


    dmc_hcmp_set_config(CurrCardNo, hcmp_0, axis, cmp_source, cmp_logic, time);//配置比较器0关联轴4，比较源为辅助编码器计数器

    dmc_hcmp_clear_points(CurrCardNo,hcmp_0);//清除比较点

    dmc_hcmp_set_mode(CurrCardNo, hcmp_0, cmp_mode_0);//配置比较器0  比较模式 队列模式

    // dmc_hcmp_set_config(CurrCardNo, hcmp_0, axis, cmp_source, cmp_logic, time);//配置比较器0关联轴0，比较源为辅助编码器计数器

    dmc_hcmp_add_point(CurrCardNo, hcmp_0, cmp_pos);//配置比较器0  添加比较点位置  cmp_pos

}

void MotionCtrlCommand::nmcClearAxisErrcode()
{
    for(int i = 0;i<7;i++)
    {
        nmc_clear_axis_errcode(CurrCardNo,i);

        // QThread::sleep(10);
    }

}

bool MotionCtrlCommand::allAxisResetHoming()
{
    QFuture<bool> future;
    future = QtConcurrent::run([=](){
        ushort state;
        for(int i = 0; i<5;i++)
        {
            axHomingMove(i);
            while(dmc_check_done(CurrCardNo,i) == 0)
            {
                QThread::usleep(10);
            }
            dmc_get_home_result(CurrCardNo,i,&state);
            if(state != 1)
            {
                qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state;
                return false;
            }
        }
        return true;
    });

    return future.result();

}


bool MotionCtrlCommand::allAxisResetHoming2()
{
    //轴0，光管
    QFuture<bool> future0;
    future0 = QtConcurrent::run([=](){
        ushort state0;
        int i = 0;
        axHomingMove(i);
        while(dmc_check_done(CurrCardNo,i) == 0)
        {
            QThread::usleep(10);
        }
        dmc_get_home_result(CurrCardNo,i,&state0);
        if(state0 != 1)
        {
            qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state0;
            return false;
        }
        return true;
    });

    //轴1，探测器
    QFuture<bool> future1;
    future1 = QtConcurrent::run([=](){
        ushort state1;
        int i = 1;
        axHomingMove(i);
        while(dmc_check_done(CurrCardNo,i) == 0)
        {
            QThread::usleep(10);
        }
        dmc_get_home_result(CurrCardNo, i, &state1);
        if(state1 != 1)
        {
            qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state1;
            return false;
        }
        return true;
    });

    //轴2，载物台Y轴
    QFuture<bool> future2;
    future2 = QtConcurrent::run([=](){
        ushort state2;
        int i = 2;
        axHomingMove(i);
        while(dmc_check_done(CurrCardNo,i) == 0)
        {
            QThread::usleep(10);
        }
            (CurrCardNo, i, &state2);
        if(state2 != 1)
        {
            qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state2;
            return false;
        }
        return true;
    });

    //轴3，载物台Z轴
    QFuture<bool> future3;
    future3 = QtConcurrent::run([=](){
        ushort state3;
        int i = 3;
        axHomingMove(i);
        while(dmc_check_done(CurrCardNo,i) == 0)
        {
            QThread::usleep(10);
        }
        dmc_get_home_result(CurrCardNo, i, &state3);
        if(state3 != 1)
        {
            qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state3;
            return false;
        }
        return true;
    });

    //轴4，载物台R轴
    QFuture<bool> future4;
    future4 = QtConcurrent::run([=](){
        ushort state4;
        int i = 4;
        axHomingMove(i);
        while(dmc_check_done(CurrCardNo,i) == 0)
        {
            QThread::usleep(10);
        }
        dmc_get_home_result(CurrCardNo, i, &state4);
        if(state4 != 1)
        {
            qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state4;
            return false;
        }
        return true;
    });


    // 创建 QFutureWatcher 对象来监视五个 QFuture 对象的状态
    QFutureWatcher<bool> watcher0;
    QFutureWatcher<bool> watcher1;
    QFutureWatcher<bool> watcher2;
    QFutureWatcher<bool> watcher3;
    QFutureWatcher<bool> watcher4;

    // 定义变量用于存储异步线程的结果
    bool result0, result1, result2, result3, result4;

    // 连接信号与槽函数，以在异步线程完成时获取计算结果
    QObject::connect(&watcher0, &QFutureWatcher<bool>::finished, [&]() {
        result0 = watcher0.result();
    });

    QObject::connect(&watcher1, &QFutureWatcher<bool>::finished, [&]() {
        result1 = watcher1.result();
    });

    QObject::connect(&watcher2, &QFutureWatcher<bool>::finished, [&]() {
        result2 = watcher2.result();
    });

    QObject::connect(&watcher3, &QFutureWatcher<bool>::finished, [&]() {
        result3 = watcher3.result();
    });

    QObject::connect(&watcher4, &QFutureWatcher<bool>::finished, [&]() {
        result4 = watcher4.result();
    });

    // 启动监视器并等待所有 QFuture 对象完成
    watcher0.setFuture(future0);
    watcher1.setFuture(future1);
    watcher2.setFuture(future2);
    watcher3.setFuture(future3);
    watcher4.setFuture(future4);

    // 等待所有异步线程完成
    watcher0.waitForFinished();
    watcher1.waitForFinished();
    watcher2.waitForFinished();
    watcher3.waitForFinished();
    watcher4.waitForFinished();

    // 根据五个异步线程的结果进行计算，获得最后的布尔值
    bool finalResult = result0 && result1 && result2 && result3 && result4;

    // 最后的布尔值可用于后续处理
    return finalResult;

}

//光管X轴，即轴 i=0
bool MotionCtrlCommand::ligntAxisResetHoming()
{
    QFuture<bool> future;
    future = QtConcurrent::run([=](){
        ushort state;

        int i = 0;
        axHomingMove(i); //回零操作

        //判断回零是否正常
        while(dmc_check_done(CurrCardNo,i) == 0)
        {
            QThread::usleep(10);
        }
        dmc_get_home_result(CurrCardNo,i,&state);
        if(state != 1)
        {
            qDebug()<<"axisNo."<<i<<" Homing failed Err Code = "<<state;
            return false;
        }

        return true;
    });

    return future.result();
}

bool MotionCtrlCommand::moveAcqSpot()
{
    QFuture<bool> future;

    future = QtConcurrent::run([=](){
        double vel;
        for(int i=0;i<5;i++)
        {
            vel = getValueFromIni(QString("MotionCtrl/absSpeed%1").arg(i+1)).toDouble();
            switch(i)
            {
            case 0:
                ptpMove(i,vel,axisAnchor1,1);
                break;
            case 1:
                ptpMove(i,vel,axisAnchor2,1);
                break;
            case 2:
                ptpMove(i,vel,axisAnchor3,1);
                break;
            case 3:
                s_CamPos = POS_LOW;
                qDebug()<<"moveAcqSpot::CamMove";
                CamScan(false);
                break;
            }
        }
        while(!dmc_check_done(CurrCardNo,axisNo1) || !dmc_check_done(CurrCardNo,axisNo2) || !dmc_check_done(CurrCardNo,axisNo3) ||
               !dmc_check_done(CurrCardNo,axisNo4) || !dmc_check_done(CurrCardNo,axisNo5))
        {
            //阻塞等待旋转轴运动到指定位置后进行下一步操作
            QThread::usleep(1);
        }
        emit sig_moveAcqSpotComplete();
        return true;
    });
    return true;
}

//光管移动到锚点(指定位置)
bool MotionCtrlCommand::moveLightToAhchor()
{
    QFuture<bool> future;

    future = QtConcurrent::run([=](){
        double vel;

        int i=0;
        vel = getValueFromIni(QString("MotionCtrl/absSpeed%1").arg(i+1)).toDouble();
        ptpMove(i,vel,axisAnchor1,1);


        while(!dmc_check_done(CurrCardNo,axisNo1))
        {
            //阻塞等待轴0运动到指定位置后进行下一步操作
            QThread::usleep(1);
        }
        emit sig_moveLightToAhchorComplete();
        return true;
    });
    return true;
}

void MotionCtrlCommand::setCameraVelocity(double vel,int angleNo)
{
    switch(angleNo)
    {
        case 1:
            cameraZ_vel_ang1=vel;
            break;
        case 2:
            cameraZ_vel_ang2 = vel;
            break;
        case 3:
            cameraZ_vel_ang3 = vel;
            break;
        case 4:
            cameraZ_vel_ang4 = vel;
            break;
    }
}

void MotionCtrlCommand::CamScan(bool loop,int angleNo)
{
    if(!loop)
    {
        double vel;
        switch(angleNo)
        {
        case 1:
            vel = cameraZ_vel_ang1;
            break;
        case 2:
            vel = cameraZ_vel_ang2;
            break;
        case 3:
            vel = cameraZ_vel_ang3;
            break;
        case 4:
            vel = cameraZ_vel_ang4;
            break;
        }
        if(!isCamScan)
        {
            while(dmc_check_done(CurrCardNo,axisNo5) == 0)
            {
                //阻塞等待旋转轴运动到指定位置后进行下一步操作
                QThread::usleep(1);
            }
            s_CamPos = POS_LOW;
            CamMove(s_CamPos,vel,vel);
            while(dmc_check_done(CurrCardNo,axisNo4) == 0)
            {
                //阻塞等待Z轴移动到最高点才进行下一步动作
                QThread::usleep(1);
            }
            emit sig_beginAcq();
            //QThread::msleep(30);
            s_CamPos = POS_HIGH;
            CamMove(s_CamPos,vel,vel);
            isCamScan = true;
        }
        isCamScan = false;
    }else
    {
//        b_scanLoop = true;
//        while(dmc_check_done(CurrCardNo,axisNo5) == 0)
//        {
//            QThread::msleep(10);
//        }
//        while(b_scanLoop.load(std::memory_order_consume))
//        {
//            while(dmc_check_done(CurrCardNo,axisNo4) == 0)
//            {
//                QThread::msleep(100);
//            }
//            qDebug()<<"CamMove";
//            s_CamPos = s_CamPos == POS_LOW ? POS_HIGH: POS_LOW;
//            CamMove(s_CamPos);
//        }
    }
}


//微调后拍图
void MotionCtrlCommand::CamScan2(bool loop,int angleNo)
{
    if(!loop)
    {
        double vel;
        vel = cameraZ_vel_ang1;//24

        if(!isCamScan)
        {
            s_CamPos = POS_LOW;
            CamMove(s_CamPos,vel,vel);
            while(dmc_check_done(CurrCardNo,axisNo4) == 0)
            {
                //阻塞等待Z轴移动到最高点才进行下一步动作
                QThread::usleep(1);
            }
            emit sig_beginAcq();
            //QThread::msleep(30);
            s_CamPos = POS_HIGH;
            CamMove(s_CamPos,vel,vel);
            isCamScan = true;
        }
        isCamScan = false;
    }
}

void MotionCtrlCommand::stopScan()
{
    //b_scanLoop.store(false,std::memory_order::memory_order_acquire);
}

void MotionCtrlCommand::CamMove(CamPos pos,double startVel,double vel,double Tacc,double Tdec,double StopVel)
{
    double _vel;

    switch(s_CamPos)
    {
    case CamPos::POS_LOW:
        res = dmc_set_profile_unit(CurrCardNo,axisNo4,startVel,vel,0.1,0.1,StopVel);
        if(res)
        {
            qDebug()<<"CamScan Move High set profile Failed"<<res;
            return;
        }
        res = dmc_pmove_unit(CurrCardNo,axisNo4,axisAnchor4_high,1);
        if(res)
        {
            qDebug()<<"CamScan Move Low Failed! Return ErrCode";
            return;
        }
        break;
    case CamPos::POS_HIGH:
        res = dmc_set_profile_unit(CurrCardNo,axisNo4,startVel,vel,0.1,0.1,StopVel);
        if(res)
        {
            qDebug()<<"CamScan Move High set profile Failed"<<res;
            return;
        }
        res = dmc_pmove_unit(CurrCardNo,axisNo4,axisAnchor4_low,1);
        if(res)
        {
            qDebug()<<"CamScan Move Low Failed! Return ErrCode";
            return;
        }
        break;
    }
}

void MotionCtrlCommand::setAngle(int angleNo)
{
    if(angleNo < 1 | angleNo > 4)
    {
        return;
    }
    double targetAngle = getValueFromIni(QString("ImagingAnchorParam/loaderAnchor_R%1").arg(angleNo)).toDouble();
    double vel = getValueFromIni("MotionCtrl/absSpeed5").toDouble();
    ptpMove(axisNo7,vel,targetAngle,1);
    while(dmc_check_done(CurrCardNo,axisNo7) ==0)
    {
        QThread::usleep(1);
    }
}

void MotionCtrlCommand::retrieveBattry()
{
    ptpMove(axisNo3,20,batteryPos,1);
    axHomingMove(axisNo7);
}

void MotionCtrlCommand::battryBack()
{
    ptpMove(axisNo3,20,axisAnchor3,1);
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
    readIODataTimer->start(200);

    // pwmOutput(2);
    //跟随
    // int axis = 5;//跟随轴号
    // int master_axis = 4;//主轴轴号
    //轴5  跟随 轴4 主轴运动
    // dmc_set_gear_follow_profile(CurrCardNo,axis,1,master_axis,1);
    // dmc_set_grant_error_protect_unit(CurrCardNo,axis,);
}

void MotionCtrlCommand::slot_stopTimerTask()
{
    qDebug() << "stop this thread " << QThread::currentThreadId();
    if(readCntStsTimer) {
        readCntStsTimer->stop();
    }
    if(readDataTimer) {
        readDataTimer->stop();
    }
}

void MotionCtrlCommand::slot_test()
{
    qDebug()<<"MotionCtrlCommand::slot_test";
}

void MotionCtrlCommand::slot_releaseIsRun()
{
    qDebug()<<"slot_releaseIsRun isRun == false";
    isRun = false;
}

void MotionCtrlCommand::updateMotionParams(const QString& configPath)
{
    if(!configSharedPointer.isNull())
    {
        configSharedPointer.reset(new QSettings(iniPath,QSettings::IniFormat));
    }
    setParams();
}

void MotionCtrlCommand::readCntStatus()
{
    ushort errCode;
    short ret = nmc_get_errcode(CurrCardNo, 2, &errCode);
    if(ret) {
        return;
    }
    // qDebug()<<"MotionCtrlCommand::readCntStatus s111111111111";
    if(errCode == 0) {
        isConnected = true;
        emit sig_connect(true);
    } else {
        isConnected = false;
        emit sig_connect(false);
    }
    // qDebug()<<"MotionCtrlCommand::readCntStatus 2111111111111";
}

void MotionCtrlCommand::readData()
{
    if(!isConnected) {
        return;
    }
    MachineStatus ms =MS_NONE;

    for (int i = 0; i < mcInfoVec.size(); ++i) {
        ushort axisNo = 0;
        switch (i) {
        case 0:
            axisNo = axisNo1;
            break;
        case 1:
            axisNo = axisNo2;
            break;
        case 2:
            axisNo = axisNo3;
            break;
        case 3:
            axisNo = axisNo4;
            break;
        case 4:
            axisNo = axisNo5;
            break;
        case 5:
            axisNo = axisNo6;
            break;
        case 6:
            axisNo = axisNo7;
            break;
        case 7:
            axisNo = axisNo8;
            break;
        case 8:
            axisNo = axisNo9;
            break;
        // case 9:
        //     axisNo = axisNo10;
        //     break;
        // case 10:
        //     axisNo = axisNo11;
        //     break;
        default:
            break;
        }
        mcInfoVec[i].axisnum = axisNo;
        // qDebug()<<"MotionCtrlCommand::readData i:"<<i;
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

        // if(axisNo == axisNo7)
        // {
        //     dmc_get_encoder_unit(CurrCardNo,axisNo,&current_pos);//读取位置
        // }else
        // {
            dmc_get_position_unit(CurrCardNo,axisNo,&current_pos);//读取位置
        // // --- 新增日志：打印从硬件读取的原始值 ---
        // if (i == AXIS::objectiveTable_X1 || i == AXIS::objectiveTable_Y1 || i == AXIS::Detector_W) {
        //     log_(QString("DATA_READ: Axis %1 hardware position = %2").arg(axisNo).arg(current_pos));
        // }
        // }
        if(axisNo == axisNo1)
        {
            mcInfoVec[i].curdist = dist0;
        }else  if(axisNo == axisNo2)
        {
            mcInfoVec[i].curdist = dist1;
        }else if(axisNo == axisNo6)
        {
            //            qDebug()<<"dist4:"<<dist4;
            mcInfoVec[i].curdist = dist4;
        }

        mcInfoVec[i].axEncPos = current_pos;

        if(mcInfoVec[i].axEncVel < 1 )
        {
            axisHomefinish =  axisHomingMoveFinish(axisNo);
        }

        mcInfoVec[i].ZeroingIsComplete =  axisHomefinish;
        // qDebug()<<"mcInfoVec "<<axisNo<<" :"<<current_pos<<" pos_limits[axisNo].first :"<<pos_limits[axisNo].first <<" pos_limits[axisNo].second: "<<pos_limits[axisNo].second;
    }

    QVariant var;
    var.setValue(mcInfoVec);
    // --- 新增日志：确认信号发射 ---
    // log_("DATA_EMIT: Emitting sig_dataChanged from MotionCtrlCommand.");
    emit sig_dataChanged(var);

}

void MotionCtrlCommand::readIOStatus()
{
    ushort yFMove;
    // readInbit(11,&yFMove);//读取摇杆io
    DIStatus distatus;
    QVector<DIStatus> DIPortVec;//输入IO

    for(int i = 0 ;i<80;i++)
    {
        readInbit(i,&yFMove);
        distatus.IO_Port = i;
        // qDebug()<<"IO status:"<<i<<" status:"<<yFMove;
        distatus.IO_Status = yFMove;
        DIPortVec.push_back(distatus);

    }
    QVariant var;
    var.setValue(DIPortVec);
    // emit sig_dataChanged(var);

    emit sig_DIStatusdataChanged(var);

}

bool MotionCtrlCommand::connect()
{

    short num = dmc_board_init();

    if(num <= 0 || num>13) {
        qDebug() <<"Don't have motionCard!";
        // ToastMsgWidget *toastMsg = new ToastMsgWidget();
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
    for(int i=0;i < 9;i++)
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

// void MotionCtrlCommand::litRed()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["RedLight"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::unlitRed()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["RedLight"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::litYellow()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["YellowLight"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     // qDebug()<<"litYellow:"<<IoPortMap["YellowLight"];
//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::unlitYellow()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["YellowLight"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::litGreen()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["GreenLight"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::unlitGreen()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["GreenLight"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::rayOpen()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["XRay"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::rayClose()
// {
//     qDebug()<<"ray close ";
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["XRay"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);

//     if(!res)
//     {
//         //add logic
//     }
// }

// void MotionCtrlCommand::Alarm()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["Alarm"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseAlarm()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["Alarm"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

void MotionCtrlCommand::openDoor()
{

    ushort state;

    readOutbit(IoPortMap["openDoor"],&state);

    qDebug()<<"openDoor"<<" port: "<< IoPortMap["openDoor"]<<" state: "<<state;

    unsigned long inport = dmc_read_outport(getCardNo(),0);
    if(state == 1)
    {
        writeOutbit(IoPortMap["openDoor"],0);

        QTimer::singleShot(1000,this,[this](){
            writeOutbit(IoPortMap["openDoor"],1);

        });
    }else if(state == 0)
    {
        writeOutbit(IoPortMap["openDoor"],1);

        QTimer::singleShot(1000,this,[this](){
            writeOutbit(IoPortMap["openDoor"],0);

        });
    }
}

// void MotionCtrlCommand::releaseOpenDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["Door"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::closeDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["Door"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCloseDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["Door"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::IOBackLockDoor(bool value)
// {

// }

// void MotionCtrlCommand::backLockDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["backLockDoor"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseBackLockDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["backLockDoor"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::leftLockDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["leftLockDoor"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseLeftLockDoor()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["leftLockDoor"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::stopIO()
// {

//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["stop"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseStopIO()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["stop"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::reset()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport & (~(1<<IoPortMap["reset"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseReset()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["reset"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// //安全继电器复位
// void MotionCtrlCommand::IOsafetyRelayReset()
// {
//     writeOutbit(IoPortMap["safetyRelayReset"],0);

//     QTimer::singleShot(1000,this,[this](){
//         writeOutbit(IoPortMap["safetyRelayReset"],1);

//     });
// }

// void MotionCtrlCommand::cylinderExtension1()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderExtension1"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releasecCylinderExtension1()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport |(1<<IoPortMap["cylinderExtension1"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderExtension2()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderExtension2"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderExtension2()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderExtension2"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderExtension3()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderExtension3"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderExtension3()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderExtension3"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderExtension4()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderExtension4"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderExtension4()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderExtension4"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderRetraction1()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderRetraction1"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderRetraction1()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderRetraction1"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderRetraction2()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderRetraction2"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderRetraction2()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderRetraction2"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderRetraction3()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderRetraction3"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderRetraction3()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderRetraction3"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::cylinderRetraction4()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport &(~(1<<IoPortMap["cylinderRetraction4"])));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

// void MotionCtrlCommand::releaseCylinderRetraction4()
// {
//     unsigned long inport = dmc_read_outport(getCardNo(),0);
//     unsigned long writeport = (inport | (1<<IoPortMap["cylinderRetraction4"]));
//     short res = dmc_write_outport(getCardNo(),0,writeport);
// }

void MotionCtrlCommand::slot_beamContiShootFinished()
{
    tasktype = TASK_NONE;
    axisStop(axisNo9,1);//dd马达紧急停止
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

void MotionCtrlCommand::beamContinuousScanning(double degSpeed,double angleRange)
{
    //scanningMove(degRange,speed,1,false);
    if(tasktype != TASK_NONE) {
        qDebug() << "task is running!";
        return;
    }
    tasktype = TASK_CONTINUINGMOVE;
    continueMoveSpeed = degSpeed;

    qDebug()<<"isRun :"<<isRun;

    taskFuture = QtConcurrent::run([=]{
        while(isRun) {
            try {
                readDDActPos(angleRange);//读取DD马达旋转轴位置 、回零、开始运动360*10位置
            } catch (std::exception& e) {
                qDebug() << "Exception: " <<e.what();
            }
            //            qDebug()<<__FUNCTION__<<" tasktype:"<<tasktype;
            if(tasktype == TASK_NONE) {
                double current_pos = 0;
                if(current_pos < 0.000001)
                    current_pos = 0.0;
                dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴
                qDebug() <<"End ddActPos "<< current_pos;
                qint64 endTime = QDateTime::currentMSecsSinceEpoch();
                qDebug() <<"End Time "<<endTime;
                qDebug() <<"move Time" <<endTime - startTime <<"ms";
                break;
            }
        }
        qDebug() << "beamContinuousScanning finished";
    });
}

std::vector<float> MotionCtrlCommand::csvAxis0Parsing(QString filename)
{
    std::vector<float> locationAndErr;
    // 打开 CSV 文件
    // QFile file("D:/study/Project/csvparsing/csvParsing/data1.csv");
    QString filepath = QApplication::applicationDirPath()+"/"+filename;
    qDebug()<<"csvAxis0Parsing:"<<filepath;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    }
    qDebug() << "打开文件";

    QTextStream in(&file);

    // 正则表达式匹配 CSV 行（支持带引号的字段）
    QRegularExpression regex("(\"[^\"]*\"|[^,]+)");

    int row = 0;

    // 逐行读取文件
    while (!in.atEnd()) {
        QString line = in.readLine(); // 读取一行

        QRegularExpressionMatchIterator matches = regex.globalMatch(line);

        QStringList fields;
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString field = match.captured(0).trimmed();
            if (field.startsWith('"') && field.endsWith('"')) {
                field = field.mid(1, field.length() - 2); // 去除引号
            }
            fields.append(field);
        }
        // qDebug()<<"fields:"<<fields.size();
        // 确保至少有两列数据
        if (fields.size() >= 0) {
            QString column1 = fields.at(0); // 第一列数据
            // qDebug() << "Column 1:" << column1 ;
            // locationAndErr[row] = column1.toFloat() ;
            locationAndErr.push_back(column1.toFloat());
            // qDebug() << "locationAndErr[row] :" << column1 ;

        }
        // qDebug()<<"row :"<<row<<" locationAndErr:"<<locationAndErr.size();
        row++;
    }

    // 关闭文件
    file.close();

    return locationAndErr;
}

void MotionCtrlCommand::beamStepScanning(float degRange, double step)
{
    if(taskFuture.isRunning()) {
        return;
    }

    qDebug()<<"beamStepScanning";

    float startAxis0 = appconfig.getStartAxis0();
    qDebug()<<"startAxis0:"<<startAxis0;

    float startAxis8 = appconfig.getStartAxis8();
    qDebug()<<" startAxis8:"<<startAxis8;

    degRange += startAxis8;

    isStepRuning = false;

    taskFuture = QtConcurrent::run([=]()->void{
        isStepRun = true;

        std::vector<float> locationVec = csvAxis0Parsing("axis0Step.csv");
        qDebug()<<"locationVec:"<<locationVec.size();

        QString absSpeedStr1,absSpeedStr9;
        double vel1,vel9;

        absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
        vel1 = absSpeedStr1.toDouble();

        absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
        vel9 = absSpeedStr9.toDouble();
        qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;

        float i = 0;
        int count = 0;
        float axis0step = 0;
        bool runFlag = false;


        for(; i <= degRange ;i+=step ) {

            // qDebug()<<"2222222222isStepRuning:"<<isStepRuning;
            if(isStepRuning)
                break;
            qDebug()<<"i:"<<i<<" startAxis8:"<<startAxis8<<" axis0step:"<<axis0step<<" startAxis0:"<<startAxis0;

            if(i == 0) {//当i = 0 时，开始走先 让两个轴回零
                // axHomingMove(axisNo9);
                // axHomingMove(axisNo1);
                // while(dmc_check_done(CurrCardNo,axisNo1) == 0)
                // {
                // }
                // while(dmc_check_done(CurrCardNo,axisNo9) == 0)
                // {
                // }
                axisDDToZero();
                while(dmc_check_done(CurrCardNo,axisNo1) == 0)
                {
                }
                while(dmc_check_done(CurrCardNo,axisNo9) == 0)
                {
                }
                QThread::msleep(3000);
                if(mcInfoVec[8].axEncPos == 0 || mcInfoVec[0].axEncPos == 0)
                    continue;
            }
            else if(i < startAxis8 && axis0step < startAxis0)//起始点位
            {


                // qDebug()<<"runFlag i:"<<i<<" startAxis8:"<<startAxis8<<" axis0step:"<<axis0step<<" startAxis0:"<<startAxis0;
                i = startAxis8;
                axis0step = startAxis0;
                this->ptpMove(axisNo9,vel9,startAxis8,1);
                this->ptpMove(axisNo1,vel1,startAxis0,1);

                while(!dmc_check_done(CurrCardNo,axisNo9))
                {

                }
                while(!dmc_check_done(CurrCardNo,axisNo1))
                {

                }
                qDebug() << "TFCommandData::sig_beamStepScanningEnd";
                emit sig_beamStepScanningEnd();

                //拍图--------
                //等待拍图完成-------
                qDebug()<<"isEmsStop:"<<isEmsStop;
                while(isRun  && !isEmsStop) {
                    if(isShootFinished) {
                        qDebug() <<"拍图完成!-------";
                        break;
                    }
                }
                isShootFinished = false;
                isPtpmove = true;
                runFlag = true;

                continue;
            }
            else{//当i ！= 0 时，当回过零时，两个轴开始走步长
                count++;
                if(count > locationVec.size())
                    return;

                axis0step += locationVec[count]  ;

                qDebug()<<"i:"<<i<<" axis0step:"<<axis0step<<" isShootFinished:"<<isShootFinished;

                this->ptpMove(axisNo9,vel9,i,1);
                this->ptpMove(axisNo1,vel1,axis0step,1);

                isPtpmove = true;
            }




            qDebug()<<"11111111isRun:"<<isRun<<" !isEmsStop:"<<!isEmsStop<<" isPtpmove:"<<isPtpmove;
            while(isRun && !isEmsStop && isPtpmove) {

                bool val = dmc_check_done(CurrCardNo,axisNo9);
                bool val1 = dmc_check_done(CurrCardNo,axisNo1);

                // qDebug()<<"val:"<<val;
                if((!val && !val1)|| (i == 0 && val)) {
                    qDebug() << "TFCommandData::sig_beamStepScanningStart";
                    emit sig_beamStepScanningStart();
                    isEmsStop = true;
                    isPtpmove = false;
                    break;
                }
            }
           qDebug()<<"2221111isRun:"<<isRun<<" isEmsStop:"<<isEmsStop<<" !isPtpmove:"<<!isPtpmove;
            while(isRun && isEmsStop && !isPtpmove) {

                bool val = dmc_check_done(CurrCardNo,axisNo9);
                bool val1 = dmc_check_done(CurrCardNo,axisNo1);

                // qDebug()<<"1111111111111111mcInfoVec[8].axEncPos:"<<mcInfoVec[8].axEncPos<<" mcInfoVec[0].axEncPos:"<<mcInfoVec[0].axEncPos;

                // if(mcInfoVec[8].axEncPos <= startAxis8 || mcInfoVec[0].axEncPos <= startAxis0)
                //     continue;

                if(val && val1){
                    qDebug()<<"val:"<<val<<" val1:"<<val1;
                    qDebug() << "TFCommandData::sig_beamStepScanningEnd";

                    emit sig_beamStepScanningEnd();
                    Sleep(900);
                    isEmsStop = false;
                    break;
                }
            }

            qDebug()<<"mcInfoVec[8].axEncPos:"<<mcInfoVec[8].axEncPos<<" mcInfoVec[0].axEncPos:"<<mcInfoVec[0].axEncPos;

            //拍图--------
            //等待拍图完成-------
            while(isRun  && !isEmsStop) {
                if(isShootFinished) {
                    qDebug() <<"拍图完成!-------";
                    break;
                }
            }
            isShootFinished = false;


            Sleep(10);
            qDebug()<<"degRange:"<<degRange<<" i:"<<i;
            if(degRange == i)
                count = 0;
        }
        if((i - step)>degRange && isEmsStop == false) {
            int currDeg = degRange - i + step;
            // this->ddAbsMove(currDeg,50,true);
            this->ptpMove(axisNo9,0.5,currDeg,1);
            // this->ddAbsStop();
            bool isStart = false;
            while(isRun && isEmsStop == false) {
                bool val =  dmc_check_done(CurrCardNo,axisNo9);
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





// 1. 替换 beamStepScanning3100A 函数

void MotionCtrlCommand::beamStepScanning3100A(int degRange, int step)
{
    if(taskFuture.isRunning()) {
        return;
    }

    taskFuture = QtConcurrent::run([=]()->void{

        isEmsStop = false;

        float startAxis_R = appconfig.getStartAxis_R();
        float startAxis_Y = appconfig.getStartAxis_Y();

        qDebug()<<"[R/Y TEST] 启动 R/Y 联动扫描 (R 轴, Y 轴)";
        qDebug()<<"[R/Y TEST] R 轴起始: "<< startAxis_R << " Y 轴起始: " << startAxis_Y;

        m_endPos = degRange + startAxis_R;
        m_step = step;

        // --- [R/Y TEST] 使用 R 轴(axisNo8) 和 Y 轴(axisNo3) 的速度 ---
        QString absSpeedStr3 = getValueFromIni("MotionCtrl/absSpeed3"); // Y 轴 (轴 2)
        double vel3 = absSpeedStr3.toDouble();
        QString absSpeedStr8 = getValueFromIni("MotionCtrl/absSpeed8"); // R 轴 (轴 7)
        double vel8 = absSpeedStr8.toDouble();
        qDebug()<<"[R/Y TEST] vel_R(axis 7):"<<vel8<<" vel_Y(axis 2):"<<vel3;


        double current_pos = 0;
        double current_posXray = 0;

        dmc_get_position_unit(CurrCardNo, axisNo8, &current_pos); // [R/Y TEST] 检查 R 轴

        photoPos = startAxis_R; // [R/Y TEST] R 轴起始位置
        photoPosX = photoPos;

        // !!! [R/Y TEST] 警告：必须替换为 R 轴的标定偏移值 !!!
        photoPosX = photoPosX - 53.4100; // !!! 占位符：使用 R 轴偏移


        if(current_pos != startAxis_R) // [R/Y TEST]
        {
            dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);
            double sod =118.966  - current_posXray;

            this->ptpMove(axisNo8, vel8, photoPos, 1); // [R/Y TEST] 移动 R 轴

            QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
            double k_GainCorrectionFactor = gainStr.toDouble();
            if (k_GainCorrectionFactor <= 0.0) {
                k_GainCorrectionFactor = 1.0;
                qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
            }
            qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

            double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            double corrected_bf = calculated_bf * k_GainCorrectionFactor;

            double currDeg = 199.9680 + corrected_bf;

            qDebug()<<"[R/Y TEST] currDeg (Y 轴位置):"<<currDeg;
            this->ptpMove(axisNo3, vel3, currDeg, 1); // [R/Y TEST] 移动 Y 轴

            while(!dmc_check_done(CurrCardNo, axisNo8)) {} // [R/Y TEST] 等待 R 轴
            while(!dmc_check_done(CurrCardNo, axisNo3)) {} // [R/Y TEST] 等待 Y 轴

            qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
            emit sig_beamStepScanningEnd();
        }else
        {
            dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);
            double sod =118.966  - current_posXray;

            this->ptpMove(axisNo8, vel8, photoPos, 1); // [R/Y TEST] 移动 R 轴

            QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
            double k_GainCorrectionFactor = gainStr.toDouble();
            if (k_GainCorrectionFactor <= 0.0) {
                k_GainCorrectionFactor = 1.0;
            }
            qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

            double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            double corrected_bf = calculated_bf * k_GainCorrectionFactor;

            // !!! [R/Y TEST] 警告：必须替换为 Y 轴的标定偏移值 (注意：原始代码此处为 206.8910) !!!
            double currDeg = 206.8910 + corrected_bf; // !!! 占位符：使用 Y 轴偏移
            qDebug()<<"增益校正因子"<<k_GainCorrectionFactor<<  "   [R/Y TEST] R 轴位置==="<< photoPos << " Y 轴位置 "<<currDeg <<"  calculateFormula 计算结果"<<calculated_bf ;

            qDebug()<<"[R/Y TEST] currDeg (Y 轴位置):"<<currDeg;
            this->ptpMove(axisNo3, vel3, currDeg, 1); // [R/Y TEST] 移动 Y 轴

            while(!dmc_check_done(CurrCardNo, axisNo8)) {} // [R/Y TEST] 等待 R 轴
            while(!dmc_check_done(CurrCardNo, axisNo3)) {} // [R/Y TEST] 等待 Y 轴

            qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
            emit sig_beamStepScanningEnd();
        }

    });
}


// 2. 替换 beamStepScanningfinishShooting 函数

void MotionCtrlCommand::beamStepScanningfinishShooting()
{
    QtConcurrent::run([=]()->void{

        qDebug()<<"isEmsStop:"<<isEmsStop;

        // --- [R/Y TEST] 使用 R 轴(axisNo8) 和 Y 轴(axisNo3) 的速度 ---
        QString absSpeedStr3 = getValueFromIni("MotionCtrl/absSpeed3"); // Y 轴 (轴 2)
        double vel3 = absSpeedStr3.toDouble();
        QString absSpeedStr8 = getValueFromIni("MotionCtrl/absSpeed8"); // R 轴 (轴 7)
        double vel8 = absSpeedStr8.toDouble();
        qDebug()<<"[R/Y TEST] vel_R(axis 7):"<<vel8<<" vel_Y(axis 2):"<<vel3;

        if(isEmsStop)
            return;

        double current_pos,start ,current_posXray= 0;

        dmc_get_position_unit(CurrCardNo, axisNo8, &current_pos); // [R/Y TEST] 检查 R 轴

        photoPos += m_step;
        photoPosX = photoPos;

        // !!! [R/Y TEST] 警告：必须替换为 R 轴的标定偏移值 !!!
        photoPosX = photoPosX - 53.4100; // !!! 占位符：使用 R 轴偏移

        if(current_pos != m_endPos)
        {
            dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);

            double sod = 118.966  - current_posXray;

            QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
            double k_GainCorrectionFactor = gainStr.toDouble();
            if (k_GainCorrectionFactor <= 0.0) {
                k_GainCorrectionFactor = 1.0;
            }
            qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

            this->ptpMove(axisNo8, vel8, photoPos, 1); // [R/Y TEST] 移动 R 轴

            double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            double corrected_bf = calculated_bf * k_GainCorrectionFactor;

            double currDeg = 199.9680 + corrected_bf;
            qDebug()<<"增益校正因子"<<k_GainCorrectionFactor<<  "   [R/Y TEST] R 轴位置==="<< photoPos << " Y 轴位置 "<<currDeg <<"  calculateFormula 计算结果"<<calculated_bf ;

            this->ptpMove(axisNo3, vel3, currDeg, 1); // [R/Y TEST] 移动 Y 轴

            while(!dmc_check_done(CurrCardNo, axisNo8)) {} // [R/Y TEST] 等待 R 轴
            while(!dmc_check_done(CurrCardNo, axisNo3)) {} // [R/Y TEST] 等待 Y 轴

            qDebug() << u8"TFCommandData::sig_beamStepScanningEnd 拍摄位置:"<<current_pos;
            emit sig_beamStepScanningEnd();
        }
    });
}















//==========================
// void MotionCtrlCommand::beamStepScanning3100A(int degRange, int step)
// {
//     if(taskFuture.isRunning()) {
//         return;
//     }

//     taskFuture = QtConcurrent::run([=]()->void{

//         isEmsStop = false;

//         float startAxis0 = appconfig.getStartAxis0();
//         qDebug()<<"startAxis0:"<<startAxis0;

//         float startAxis8 = appconfig.getStartAxis8();
//         qDebug()<<" startAxis8:"<<startAxis8;
//         m_endPos = degRange + startAxis8;
//         m_step = step;
//         QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
//         double vel1 = absSpeedStr1.toDouble();

//         QString absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
//         double vel9 = absSpeedStr9.toDouble();
//         qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;


//         double current_pos = 0;
//         double current_posXray = 0;

//         dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

//         photoPos = startAxis8;

//         photoPosX = startAxis8;
//         photoPosX = photoPosX - 347.4150;


//         if(current_pos != startAxis8)
//         {
//             dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);//dd马达旋转轴

//             double sod =118.966  - current_posXray; /*130  - current_posXray;*///228.5 + 载物台 X 轴

//             this->ptpMove(axisNo9,vel9,photoPos,1);
//             QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
//             double k_GainCorrectionFactor = gainStr.toDouble();
//             // 安全检查，防止配置缺失或为0，默认为 1.0 (无校正)
//             if (k_GainCorrectionFactor <= 0.0) {
//                 k_GainCorrectionFactor = 1.0;
//                 qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
//             }
//             qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

//             double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             double corrected_bf = calculated_bf * k_GainCorrectionFactor;
//             double currDeg = 205.4820 + corrected_bf;

//             // double currDeg =  205.4820 + calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             qDebug()<<"currDeg:"<<currDeg;
//             this->ptpMove(axisNo1,vel1,currDeg,1);
//             // bool val = dmc_check_done(CurrCardNo,axisNo9);

//             while(!dmc_check_done(CurrCardNo,axisNo9))
//             {

//             }
//             while(!dmc_check_done(CurrCardNo,axisNo1))
//             {

//             }
//             // bool val1 = dmc_check_done(CurrCardNo,axisNo1);//：0：指定轴正在运行，1：指定轴已停止
//             // if(val1){

//             // }
//             qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
//             emit sig_beamStepScanningEnd();
//         }else
//         {
//             double sod =118.966  - current_posXray; /* 130  - current_posXray;*///228.5 + 载物台 X 轴
//             QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
//             double k_GainCorrectionFactor = gainStr.toDouble();
//             // 安全检查，防止配置缺失或为0，默认为 1.0 (无校正)
//             if (k_GainCorrectionFactor <= 0.0) {
//                 k_GainCorrectionFactor = 1.0;
//                 qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
//             }
//             qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

//             this->ptpMove(axisNo9,vel9,photoPos,1);
//             double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             double corrected_bf = calculated_bf * k_GainCorrectionFactor;
//             double currDeg = 206.8910 + corrected_bf;

//             // double currDeg = 205.4820 +  calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             qDebug()<<"currDeg:"<<currDeg;
//             this->ptpMove(axisNo1,vel1,currDeg,1);
//             // bool val = dmc_check_done(CurrCardNo,axisNo9);

//             while(!dmc_check_done(CurrCardNo,axisNo9))
//             {

//             }
//             while(!dmc_check_done(CurrCardNo,axisNo1))
//             {

//             }
//             // bool val1 = dmc_check_done(CurrCardNo,axisNo1);//：0：指定轴正在运行，1：指定轴已停止
//             // if(val1){

//             // }
//             qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
//             emit sig_beamStepScanningEnd();
//         }

//     });
// }

// void MotionCtrlCommand::beamStepScanningfinishShooting()
// {
//     QtConcurrent::run([=]()->void{

//         qDebug()<<"isEmsStop:"<<isEmsStop;
//         QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
//         double vel1 = absSpeedStr1.toDouble();

//         QString absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
//         double vel9 = absSpeedStr9.toDouble();
//         qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;


//         if(isEmsStop)
//             return;

//         double current_pos,start ,current_posXray= 0;

//         dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

//         photoPos += m_step;
//         photoPosX = photoPos;

//         photoPosX = photoPosX - 347.4150;
//         if(current_pos != m_endPos)
//         {
//             dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);//dd马达旋转轴

//             double sod = 118.966  - current_posXray;/*130  - current_posXray;*///228.5 + 载物台 X 轴
//             // 从 INI 文件读取增益校正因子
//             QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
//             double k_GainCorrectionFactor = gainStr.toDouble();
//             // 安全检查，防止配置缺失或为0，默认为 1.0 (无校正)
//             if (k_GainCorrectionFactor <= 0.0) {
//                 k_GainCorrectionFactor = 1.0;
//                 qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
//             }
//             qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

//             this->ptpMove(axisNo9,vel9,photoPos,1);
//             double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             double corrected_bf = calculated_bf * k_GainCorrectionFactor;
//             double currDeg = 205.4820 + corrected_bf;
//             // double currDeg = 205.4820 + calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             qDebug()<<"软件增益系数==="<<k_GainCorrectionFactor<< "  当前theta轴位置 == "<< photoPos << " 当前X轴位置  "<<currDeg <<"  calculateFormula 计算结果"<<calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
//             qDebug()<<"~~ currDeg:"<<currDeg;
//             this->ptpMove(axisNo1,vel1,currDeg,1);

//             while(!dmc_check_done(CurrCardNo,axisNo9))
//             {

//             }
//             while(!dmc_check_done(CurrCardNo,axisNo1))
//             {

//             }
//             qDebug() << u8"TFCommandData::sig_beamStepScanningEnd 拍摄位置:"<<current_pos;
//             emit sig_beamStepScanningEnd();
//         }
//     });
// }

double MotionCtrlCommand::calculateFormula(double r, double sod, double d, double s)
{
    qDebug()<<"MotionCtrlCommand::calculateFormula r:"<<r<<" sod:"<<sod<<" d:"<<d<<" s:"<<s;
    if (!validateInput(r, sod, d, s)) {
        qWarning() << "Invalid input! Calculation aborted.";
        return qQNaN(); // 返回 NaN（非数字）
    }

    // 计算公式: (r*sod*sin(s/r))/(d+sod+r*cos(s/r))
    double denominator = d + sod + r * qCos(s / (r));
    if (qFuzzyIsNull(denominator)) {  // 避免除以零
        qWarning() << "Division by zero error!";
        return qQNaN();
    }


    double numerator = r * sod * qSin( s /(r));
    qDebug()<<"denominator:"<<denominator<<"  numerator:"<<numerator<<" qSin(s / r):"<<r * qSin(s / r)<<" qCos(s / r):"<<r * qCos(s / r);
    qDebug()<<numerator / denominator<<"=======numerator / denominator";
    return numerator/ denominator;
}



/*20251031函数备份=theta轴和X轴联动代码


//==========================
void MotionCtrlCommand::beamStepScanning3100A(int degRange, int step)
{
    if(taskFuture.isRunning()) {
        return;
    }

    taskFuture = QtConcurrent::run([=]()->void{

        isEmsStop = false;

        float startAxis0 = appconfig.getStartAxis0();
        qDebug()<<"startAxis0:"<<startAxis0;

        float startAxis8 = appconfig.getStartAxis8();
        qDebug()<<" startAxis8:"<<startAxis8;
        m_endPos = degRange + startAxis8;
        m_step = step;
        QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
        double vel1 = absSpeedStr1.toDouble();

        QString absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
        double vel9 = absSpeedStr9.toDouble();
        qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;


        double current_pos = 0;
        double current_posXray = 0;

        dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

        photoPos = startAxis8;

        photoPosX = startAxis8;
        photoPosX = photoPosX - 347.4150;


        if(current_pos != startAxis8)
        {
            dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);//dd马达旋转轴

            double sod =118.966  - current_posXray; //228.5 + 载物台 X 轴

            this->ptpMove(axisNo9,vel9,photoPos,1);
            QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
            double k_GainCorrectionFactor = gainStr.toDouble();
            // 安全检查，防止配置缺失或为0，默认为 1.0 (无校正)
            if (k_GainCorrectionFactor <= 0.0) {
                k_GainCorrectionFactor = 1.0;
                qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
            }
            qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

            double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            double corrected_bf = calculated_bf * k_GainCorrectionFactor;
            double currDeg = 205.4820 + corrected_bf;

            // double currDeg =  205.4820 + calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            qDebug()<<"currDeg:"<<currDeg;
            this->ptpMove(axisNo1,vel1,currDeg,1);
            // bool val = dmc_check_done(CurrCardNo,axisNo9);

            while(!dmc_check_done(CurrCardNo,axisNo9))
            {

            }
            while(!dmc_check_done(CurrCardNo,axisNo1))
            {

            }
            // bool val1 = dmc_check_done(CurrCardNo,axisNo1);//：0：指定轴正在运行，1：指定轴已停止
            // if(val1){

            // }
            qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
            emit sig_beamStepScanningEnd();
        }else
        {
            double sod =118.966  - current_posXray;//228.5 + 载物台 X 轴
            QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
            double k_GainCorrectionFactor = gainStr.toDouble();
            // 安全检查，防止配置缺失或为0，默认为 1.0 (无校正)
            if (k_GainCorrectionFactor <= 0.0) {
                k_GainCorrectionFactor = 1.0;
                qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
            }
            qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

            this->ptpMove(axisNo9,vel9,photoPos,1);
            double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            double corrected_bf = calculated_bf * k_GainCorrectionFactor;
            double currDeg = 206.8910 + corrected_bf;

            // double currDeg = 205.4820 +  calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            qDebug()<<"currDeg:"<<currDeg;
            this->ptpMove(axisNo1,vel1,currDeg,1);
            // bool val = dmc_check_done(CurrCardNo,axisNo9);

            while(!dmc_check_done(CurrCardNo,axisNo9))
            {

            }
            while(!dmc_check_done(CurrCardNo,axisNo1))
            {

            }
            // bool val1 = dmc_check_done(CurrCardNo,axisNo1);//：0：指定轴正在运行，1：指定轴已停止
            // if(val1){

            // }
            qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
            emit sig_beamStepScanningEnd();
        }

    });
}

void MotionCtrlCommand::beamStepScanningfinishShooting()
{
    QtConcurrent::run([=]()->void{

        qDebug()<<"isEmsStop:"<<isEmsStop;
        QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
        double vel1 = absSpeedStr1.toDouble();

        QString absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
        double vel9 = absSpeedStr9.toDouble();
        qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;


        if(isEmsStop)
            return;

        double current_pos,start ,current_posXray= 0;

        dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

        photoPos += m_step;
        photoPosX = photoPos;

        photoPosX = photoPosX - 347.4150;
        if(current_pos != m_endPos)
        {
            dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);//dd马达旋转轴

            double sod = 118.966  - current_posXray;/*130  - current_posXray;//228.5 + 载物台 X 轴
            // 从 INI 文件读取增益校正因子
            QString gainStr = getValueFromIni("MotionCtrl/gainFactor");
            double k_GainCorrectionFactor = gainStr.toDouble();
            // 安全检查，防止配置缺失或为0，默认为 1.0 (无校正)
            if (k_GainCorrectionFactor <= 0.0) {
                k_GainCorrectionFactor = 1.0;
                qWarning() << "MotionCtrl/gainFactor not found or invalid, defaulting to 1.0";
            }
            qDebug() << "Using Gain Correction Factor:" << k_GainCorrectionFactor;

            this->ptpMove(axisNo9,vel9,photoPos,1);
            double calculated_bf = calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            double corrected_bf = calculated_bf * k_GainCorrectionFactor;
            double currDeg = 205.4820 + corrected_bf;
            // double currDeg = 205.4820 + calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            qDebug()<<"软件增益系数==="<<k_GainCorrectionFactor<< "  当前theta轴位置 == "<< photoPos << " 当前X轴位置  "<<currDeg <<"  calculateFormula 计算结果"<<calculateFormula(AppConfig::R, sod, AppConfig::d0, photoPosX);
            qDebug()<<"~~ currDeg:"<<currDeg;
            this->ptpMove(axisNo1,vel1,currDeg,1);

            while(!dmc_check_done(CurrCardNo,axisNo9))
            {

            }
            while(!dmc_check_done(CurrCardNo,axisNo1))
            {

            }
            qDebug() << u8"TFCommandData::sig_beamStepScanningEnd 拍摄位置:"<<current_pos;
            emit sig_beamStepScanningEnd();
        }
    });
}

double MotionCtrlCommand::calculateFormula(double r, double sod, double d, double s)
{
    qDebug()<<"MotionCtrlCommand::calculateFormula r:"<<r<<" sod:"<<sod<<" d:"<<d<<" s:"<<s;
    if (!validateInput(r, sod, d, s)) {
        qWarning() << "Invalid input! Calculation aborted.";
        return qQNaN(); // 返回 NaN（非数字）
    }

    // 计算公式: (r*sod*sin(s/r))/(d+sod+r*cos(s/r))
    double denominator = d + sod + r * qCos(s / (r));
    if (qFuzzyIsNull(denominator)) {  // 避免除以零
        qWarning() << "Division by zero error!";
        return qQNaN();
    }


    double numerator = r * sod * qSin( s /(r));
    qDebug()<<"denominator:"<<denominator<<"  numerator:"<<numerator<<" qSin(s / r):"<<r * qSin(s / r)<<" qCos(s / r):"<<r * qCos(s / r);
    qDebug()<<numerator / denominator<<"=======numerator / denominator";
    return numerator/ denominator;
}


**/


// 20251029函数备份
// {
//     void MotionCtrlCommand::beamStepScanning3100A(int degRange, int step)
//     {
//         if(taskFuture.isRunning()) {
//             return;
//         }

//         taskFuture = QtConcurrent::run([=]()->void{

//             isEmsStop = false;

//             float startAxis0 = appconfig.getStartAxis0();
//             qDebug()<<"startAxis0:"<<startAxis0;

//             float startAxis8 = appconfig.getStartAxis8();
//             qDebug()<<" startAxis8:"<<startAxis8;
//             m_endPos = degRange + startAxis8;
//             m_step = step;
//             QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
//             double vel1 = absSpeedStr1.toDouble();

//             QString absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
//             double vel9 = absSpeedStr9.toDouble();
//             qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;




//             double current_pos = 0;
//             double current_posXray = 0;

//             dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

//             photoPos = startAxis8;

//             photoPosX = startAxis8;
//             photoPosX = fabs(photoPosX - 340.388);


//             if(current_pos != startAxis8)
//             {
//                 dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);//dd马达旋转轴

//                 double sod = 130  - current_posXray;//228.5 + 载物台 X 轴

//                 this->ptpMove(axisNo9,vel9,photoPos,1);
//                 double currDeg = calculateFormula(AppConfig::R, sod, AppConfig::totalLen-AppConfig::R, photoPosX);
//                 qDebug()<<"currDeg:"<<currDeg;
//                 this->ptpMove(axisNo1,vel1,currDeg,1);
//                 // bool val = dmc_check_done(CurrCardNo,axisNo9);

//                 while(!dmc_check_done(CurrCardNo,axisNo9))
//                 {

//                 }
//                 while(!dmc_check_done(CurrCardNo,axisNo1))
//                 {

//                 }
//                 // bool val1 = dmc_check_done(CurrCardNo,axisNo1);//：0：指定轴正在运行，1：指定轴已停止
//                 // if(val1){

//                 // }
//                 qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
//                 emit sig_beamStepScanningEnd();
//             }else
//             {
//                 double sod = 130  - current_posXray;//228.5 + 载物台 X 轴

//                 this->ptpMove(axisNo9,vel9,photoPos,1);
//                 double currDeg = calculateFormula(AppConfig::R, sod, AppConfig::totalLen-AppConfig::R, photoPosX);
//                 qDebug()<<"currDeg:"<<currDeg;
//                 this->ptpMove(axisNo1,vel1,currDeg,1);
//                 // bool val = dmc_check_done(CurrCardNo,axisNo9);

//                 while(!dmc_check_done(CurrCardNo,axisNo9))
//                 {

//                 }
//                 while(!dmc_check_done(CurrCardNo,axisNo1))
//                 {

//                 }
//                 // bool val1 = dmc_check_done(CurrCardNo,axisNo1);//：0：指定轴正在运行，1：指定轴已停止
//                 // if(val1){

//                 // }
//                 qDebug() << u8"第一次拍摄TFCommandData::sig_beamStepScanningEnd";
//                 emit sig_beamStepScanningEnd();
//             }

//         });
//     }

//     void MotionCtrlCommand::beamStepScanningfinishShooting()
//     {
//         QtConcurrent::run([=]()->void{

//             qDebug()<<"isEmsStop:"<<isEmsStop;
//             QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
//             double vel1 = absSpeedStr1.toDouble();

//             QString absSpeedStr9 = getValueFromIni("MotionCtrl/absSpeed9");
//             double vel9 = absSpeedStr9.toDouble();
//             qDebug()<<"vel1:"<<vel1<<" vel9:"<<vel9;

//             if(isEmsStop)
//                 return;

//             double current_pos,start ,current_posXray= 0;

//             dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

//             photoPos += m_step;
//             photoPosX = photoPos;

//             photoPosX = fabs(photoPosX - 340.388);
//             if(current_pos != m_endPos)
//             {
//                 dmc_get_position_unit(CurrCardNo,AXIS::XRAY_Z,&current_posXray);//dd马达旋转轴

//                 double sod = 130  - current_posXray;//228.5 + 载物台 X 轴

//                 this->ptpMove(axisNo9,vel9,photoPos,1);
//                 double currDeg = calculateFormula(AppConfig::R, sod, AppConfig::totalLen-AppConfig::R, photoPosX);
//                 qDebug()<<"~~ currDeg:"<<currDeg;
//                 this->ptpMove(axisNo1,vel1,currDeg,1);

//                 while(!dmc_check_done(CurrCardNo,axisNo9))
//                 {

//                 }
//                 while(!dmc_check_done(CurrCardNo,axisNo1))
//                 {

//                 }
//                 qDebug() << u8"TFCommandData::sig_beamStepScanningEnd 拍摄位置:"<<current_pos;
//                 emit sig_beamStepScanningEnd();
//             }
//         });
//     }

//     double MotionCtrlCommand::calculateFormula(double r, double sod, double d, double s)
//     {
//         qDebug()<<"MotionCtrlCommand::calculateFormula r:"<<r<<" sod:"<<sod<<" d:"<<d<<" s:"<<s;
//         if (!validateInput(r, sod, d, s)) {
//             qWarning() << "Invalid input! Calculation aborted.";
//             return qQNaN(); // 返回 NaN（非数字）
//         }

//         // 计算公式: (r*sod*sin(s/r))/(d+sod+r*cos(s/r))
//         double denominator = d + sod + r * qCos(s / r);
//         if (qFuzzyIsNull(denominator)) {  // 避免除以零
//             qWarning() << "Division by zero error!";
//             return qQNaN();
//         }


//         double numerator = r * sod * qSin(s / r);
//         qDebug()<<"denominator:"<<denominator<<"  numerator:"<<numerator<<" qSin(s / r):"<<r * qSin(s / r)<<" qCos(s / r):"<<r * qCos(s / r);

//         return numerator / denominator;
//     }

// }






bool MotionCtrlCommand::validateInput(double r, double sod, double d, double s)
{
    // 检查 r 是否为 0（避免 s/r 除零错误）
    if (qFuzzyIsNull(r)) {
        qWarning() << "Error: r cannot be zero.";
        return false;
    }

    // 检查其他输入是否合法（例如非负数等，根据实际需求调整）
    if (sod < 0 || d < 0) {
        qWarning() << "Error: sod and d must be non-negative.";
        return false;
    }

    return true;
}

void MotionCtrlCommand::slot_beamEmsStop3100A()
{
    tasktype = TASK_NONE;
    isEmsStop = true;
    // if(!isStepRun) {
    //     isEmsStop = false;
    // }
    qDebug()<<"slot_beamEmsStop isEmsStop:"<<isEmsStop;
    this->axisStop(axisNo9,1);
    this->axisStop(axisNo1,1);
    this->axisStop(axisNo8, 1); // 停止 R 轴 (轴 7)
    this->axisStop(axisNo3, 1); // 停止 Y 轴 (轴 2)
    // this->emgStop();
}
//========================================
//一维高速位置比较功能
void MotionCtrlCommand::HcmpBeamContinuousScanning(double degSpeed)
{
    //scanningMove(degRange,speed,1,false);
    if(tasktype != TASK_NONE) {
        qDebug() << "task is running!";
        return;
    }
    tasktype = TASK_CONTINUINGMOVE;
    continueMoveSpeed = degSpeed;

    qDebug()<<"isRun :"<<isRun;

    taskFuture = QtConcurrent::run([=]{

        // HcmpSetMode(0);//一维高速比较 1高电平 0 低电平
        // HcmpSetMode(1);//一维高速比较 1高电平 0 低电平

        while(isRun) {
            try {
                HcmpReadDDActPos();//读取DD马达旋转轴位置 、回零、开始运动360*10位置

            } catch (std::exception& e) {
                qDebug() << "Exception: " <<e.what();
            }
            //            qDebug()<<__FUNCTION__<<" tasktype:"<<tasktype;
            if(tasktype == TASK_NONE) {
                double current_pos = 0;
                if(current_pos < 0.000001)
                    current_pos = 0.0;
                dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴
                qDebug() <<"End ddActPos "<< current_pos;
                qint64 endTime = QDateTime::currentMSecsSinceEpoch();
                qDebug() <<"End Time "<<endTime;
                qDebug() <<"move Time" <<endTime - startTime <<"ms";
                break;
            }
        }
        qDebug() << "beamContinuousScanning finished";
    });
}

void MotionCtrlCommand::HcmpReadDDActPos()
{
    double current_pos = 0;
    if(current_pos < 0.000001)
        current_pos = 0.0;
    dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴

    // qDebug()<<"dmc_get_position_unit current_pos:"<<current_pos;
    // mcInfoVec[i].axEncPos = current_pos;
    // 判断是否需要进行回零
    if(tasktype == TASK_CONTINUINGMOVE)
    {
        if(qFuzzyCompare(current_pos,0))
        {
            tasktype = TASK_CONTINUINGMOVE_START;
            qDebug() << "TASK_CONTINUINGMOVE_START";
        }
        else
        {
            tasktype = TASK_CONTINUINGMOVE_HOME;
            qDebug() << "TASK_CONTINUINGMOVE_HOME";
        }
        // emit sig_beamContiScanningStart();
    }
    //回零任务
    if(tasktype == TASK_CONTINUINGMOVE_HOME) {
        static bool resetMove = false;
        if(qFuzzyCompare(current_pos,0))
        {
            while(dmc_check_done(CurrCardNo,axisNo9) == 1)
            {
                QThread::usleep(10);
                tasktype = TASK_CONTINUINGMOVE_START;
                qDebug() << "TASK_CONTINUINGMOVE_START";
                resetMove = false;
                break;
            }

        }
        else
        {
            if(resetMove) {
                return;
            }
            qDebug() << "homeMove";
            resetMove = true;
            axHomingMove(axisNo9);


        }
    }
    //开始移动
    if(tasktype == TASK_CONTINUINGMOVE_START)
    {
        tasktype = TASK_CONTINUINGMOVE_SHOOT;
        if(qFuzzyCompare(current_pos,0))
        {
            qDebug() << "TASK_CONTINUINGMOVE_RUNING------";
            int range = 360/*00*/;
            //        continueMoveSpeed = 50;
            //        range = 3600;
            // continueMoveSpeed = 20;
            ptpMove(axisNo9,continueMoveSpeed,range,1);

            // QTimer::singleShot(1000,this,[this]{this->axisStop(axisNo7,1);});
        }
    }
    //移动至拍照位
    if(tasktype == TASK_CONTINUINGMOVE_SHOOT) {
        //        qDebug() <<__FUNCTION__<< "start ddActPos  "<< ddActPos;
        // HcmpSetMode();//一维高速比较
        ushort state=-1,state1 = -1,state2 = -1,state3 = -1;
        readInbit(2,&state);//一维高速比较连接端口

        if(current_pos <3)
        {
             qDebug()<<"一维高速比较 state:"<<state<<" io3 "<<state1<<" io1 "<<state2<<" io0 "<<state3<<" current_pos:"<<current_pos;
        }
        if(state == 0)
        {
            startTime = QDateTime::currentMSecsSinceEpoch();
            //            qDebug() <<__FUNCTION__<< "start ddActPos  "<< ddActPos;
            qDebug() <<__FUNCTION__<< "start Time  "<< startTime;
            tasktype = TASK_CONTINUINGMOVE_RUNING;
            qDebug() << "TASK_CONTINUINGMOVE_RUNING";
            emit sig_allowBeamShooting();//触发拍照   软触发

            //            qDebug() << "sig_allowBeamShooting";
        }
    }
    if(tasktype == TASK_CONTINUINGMOVE_RUNING) {
        //        if(ddActPos >= 900.0)
        //        {
        //            qDebug() << "slot_beamContiShootFinished";
        //            this->slot_beamContiShootFinished();
        //        }
    }

    if(current_pos > 360 && TASK_CONTINUINGMOVE_RUNING)
    {
        qDebug()<<"11111111111";
        // HcmpSetMode();
    }

}

void MotionCtrlCommand::readDDActPos(double angleRange)
{
    // qDebug()<<"MotionCtrlCommand::readDDActPos angleRange:"<<angleRange;
    double current_pos = 0;
    if(current_pos < 0.000001)
        current_pos = 0.0;
    dmc_get_position_unit(CurrCardNo,axisNo9,&current_pos);//dd马达旋转轴
    // qDebug()<<"dmc_get_position_unit current_pos:"<<current_pos;
    // mcInfoVec[i].axEncPos = current_pos;
    // 判断是否需要进行回零
    if(tasktype == TASK_CONTINUINGMOVE)
    {
        if(qFuzzyCompare(current_pos,0))
        {
            tasktype = TASK_CONTINUINGMOVE_START;
            qDebug() << "TASK_CONTINUINGMOVE_START";
        }
        else
        {
            tasktype = TASK_CONTINUINGMOVE_HOME;
            qDebug() << "TASK_CONTINUINGMOVE_HOME";
        }
        // emit sig_beamContiScanningStart();
    }
    //回零任务
    if(tasktype == TASK_CONTINUINGMOVE_HOME) {
        static bool resetMove = false;
        if(qFuzzyCompare(current_pos,0))
        {
            while(dmc_check_done(CurrCardNo,axisNo9) == 1)
            {
                QThread::usleep(10);
                tasktype = TASK_CONTINUINGMOVE_START;
                qDebug() << "TASK_CONTINUINGMOVE_START";
                resetMove = false;
                break;
            }

        }
        else
        {
            if(resetMove) {
                return;
            }
            qDebug() << "homeMove";
            resetMove = true;
            // axHomingMove(axisNo7);
            axisDDToZero();

        }
    }
    //开始移动
    if(tasktype == TASK_CONTINUINGMOVE_START)
    {
        tasktype = TASK_CONTINUINGMOVE_SHOOT;
        if(qFuzzyCompare(current_pos,0))
        {
            qDebug() << "TASK_CONTINUINGMOVE_RUNING------";
            int range = angleRange+50/*00*/;
            //        continueMoveSpeed = 50;
            //        range = 3600;
            // continueMoveSpeed = 20;
            ptpMove(axisNo9,continueMoveSpeed,range,1);

            // QTimer::singleShot(1000,this,[this]{this->axisStop(axisNo7,1);});
        }
    }
    //移动至拍照位
    if(tasktype == TASK_CONTINUINGMOVE_SHOOT) {
        //        qDebug() <<__FUNCTION__<< "start ddActPos  "<< ddActPos;
        if(current_pos >= 50.0)
        {
            startTime = QDateTime::currentMSecsSinceEpoch();
            //            qDebug() <<__FUNCTION__<< "start ddActPos  "<< ddActPos;
            qDebug() <<__FUNCTION__<< "start Time  "<< startTime;
            tasktype = TASK_CONTINUINGMOVE_RUNING;
            qDebug() << "TASK_CONTINUINGMOVE_RUNING";
            emit sig_allowBeamShooting();//触发拍照   软触发

            //            qDebug() << "sig_allowBeamShooting";
        }
    }
    if(tasktype == TASK_CONTINUINGMOVE_RUNING) {
        //        if(ddActPos >= 900.0)
        //        {
        //            qDebug() << "slot_beamContiShootFinished";
        //            this->slot_beamContiShootFinished();
        //        }
    }
}

void MotionCtrlCommand::slot_beamEmsStop()
{
    tasktype = TASK_NONE;
    isEmsStop = true;
    isStepRuning = true;

    if(!isStepRun) {
        isEmsStop = false;
    }
    qDebug()<<"slot_beamEmsStop isEmsStop:"<<isEmsStop;
    this->axisStop(axisNo9,1);
    this->axisStop(axisNo1,1);
    this->emgStop();
}

//电池夹具旋转轴  运动角度
void MotionCtrlCommand::slot_batteryClampRotates(float angle)
{
    QString absSpeedStr = getValueFromIni("MotionCtrl/absSpeed10");
    double vel = absSpeedStr.toDouble();

    ptpMove(axisNo10,vel,angle,1);//
}
//所有轴回零
bool MotionCtrlCommand::axisAllHomingMove()
{
    axHomingMove(0);
    axHomingMove(5);
    axHomingMove(6);
    axHomingMove(1);

    axHomingMove(2);
    axHomingMove(3);
    axHomingMove(7);
    axHomingMove(4);

    return true;
}
void MotionCtrlCommand::axisDDToZero()
{
    QString absSpeedStr = getValueFromIni("MotionCtrl/absSpeed9");
    double vel = absSpeedStr.toDouble();

    ptpMove(axisNo9,vel,0,1);

    QString absSpeedStr1 = getValueFromIni("MotionCtrl/absSpeed1");
    double vel1 = absSpeedStr1.toDouble();

    ptpMove(axisNo1,vel1,0,1);
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
                qDebug()<<"axis 4 :"<<currLocation - err;
                return currLocation - err;

            }
        }

    }
}

void MotionCtrlCommand::setParams()
{
    axisNo1 = getValueFromIni("MotionCtrl/axisNo1").toInt();
    axisNo2 = getValueFromIni("MotionCtrl/axisNo2").toInt();
    axisNo3 = getValueFromIni("MotionCtrl/axisNo3").toInt();
    axisNo4 = getValueFromIni("MotionCtrl/axisNo4").toInt();
    axisNo5 = getValueFromIni("MotionCtrl/axisNo5").toInt();
    axisNo6 = getValueFromIni("MotionCtrl/axisNo6").toInt();
    axisNo7 = getValueFromIni("MotionCtrl/axisNo7").toInt();
    axisNo8 = getValueFromIni("MotionCtrl/axisNo8").toInt();
    axisNo9 = getValueFromIni("MotionCtrl/axisNo9").toInt();
    axisNo10 = getValueFromIni("MotionCtrl/axisNo10").toInt();
    axisNo11 = getValueFromIni("MotionCtrl/axisNo11").toInt();
    axisNo12 = getValueFromIni("MotionCtrl/axisNo12").toInt();
    axisNo13 = getValueFromIni("MotionCtrl/axisNo13").toInt();

    cameraZ_vel_ang1 = getValueFromIni("ImagingAnchorParam/cameraVel_ang1").toDouble();
    cameraZ_vel_ang2 = getValueFromIni("ImagingAnchorParam/cameraVel_ang2").toDouble();
    cameraZ_vel_ang3 = getValueFromIni("ImagingAnchorParam/cameraVel_ang3").toDouble();
    cameraZ_vel_ang4 = getValueFromIni("ImagingAnchorParam/cameraVel_ang4").toDouble();

    qDebug()<<"cameraZ_vel_ang1"<<cameraZ_vel_ang1<<"cameraZ_vel_ang2"<<cameraZ_vel_ang2<<"cameraZ_vel_ang3"<<cameraZ_vel_ang3<<"cameraZ_vel_ang4"<<cameraZ_vel_ang4;
    axisAnchor1 = getValueFromIni("ImagingAnchorParam/rayAnchor_X").toDouble();
    axisAnchor2 = getValueFromIni("ImagingAnchorParam/probeAnchor_X").toDouble();
    axisAnchor3 = getValueFromIni("ImagingAnchorParam/loaderAnchor_Y").toDouble();
    axisAnchor4_low = getValueFromIni("ImagingAnchorParam/loaderAnchor_Z1").toDouble();
    axisAnchor4_high = getValueFromIni("ImagingAnchorParam/loaderAnchor_Z2").toDouble();
    batteryPos = getValueFromIni("ImagingAnchorParam/loaderY_rollbackPos").toDouble();

    pos_limits[axisNo1] = qMakePair(
        getValueFromIni("ImagingAnchorParam/probeZ_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/probeZ_pLimit").toDouble());//探测板Z
    pos_limits[axisNo2] = qMakePair(
        getValueFromIni("ImagingAnchorParam/rayZ_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/rayZ_pLimit").toDouble());//光管Z
    pos_limits[axisNo3] = qMakePair(
        getValueFromIni("ImagingAnchorParam/loaderUY_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/loaderUY_pLimit").toDouble());//旋转平台U-y
    pos_limits[axisNo4] = qMakePair(
        getValueFromIni("ImagingAnchorParam/loaderDY_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/loaderDY_pLimit").toDouble());//旋转平台D-y
    pos_limits[axisNo6] = qMakePair(
        getValueFromIni("ImagingAnchorParam/probeY_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/probeY_pLimit").toDouble());//探测板Y
    pos_limits[axisNo7] = qMakePair(
        getValueFromIni("ImagingAnchorParam/probeX_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/probeX_pLimit").toDouble());//探测板X
    pos_limits[axisNo8] = qMakePair(
        getValueFromIni("ImagingAnchorParam/loaderX_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/loaderX_pLimit").toDouble());//旋转平台X
    pos_limits[axisNo11] = qMakePair(
        getValueFromIni("ImagingAnchorParam/DRprobey_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/DRprobey_pLimit").toDouble());
    pos_limits[axisNo12] = qMakePair(
        getValueFromIni("ImagingAnchorParam/DRloadery_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/DRloadery_pLimit").toDouble());
    pos_limits[axisNo13] = qMakePair(
        getValueFromIni("ImagingAnchorParam/DRZ_nLimit").toDouble(),
        getValueFromIni("ImagingAnchorParam/DRZ_pLimit").toDouble());

}

QString MotionCtrlCommand::getValueFromIni(QString str)
{
    if(configSharedPointer.isNull()) {
        return "";
    }
    return configSharedPointer->value(str).toString();
}

void MotionCtrlCommand::setValue2Ini(QString str, QString value)
{
    if(configSharedPointer.isNull()) {
        return;
    }
    configSharedPointer->setValue(str,value);
}

axisPos08 MotionCtrlCommand::axisDDPos()
{
    axisPos08 axis;
    double current_pos = 0;
    dmc_get_position_unit(CurrCardNo,objectiveTable_X1,&current_pos);//dd马达旋转轴
    axis.objectiveTableXPos = current_pos;

    dmc_get_position_unit(CurrCardNo,objectiveTable_Y1,&current_pos);//dd马达旋转轴
    axis.objectiveTableYPos = current_pos;

    dmc_get_position_unit(CurrCardNo,Detector_R,&current_pos);//dd马达旋转轴
    axis.Detector_RPos = current_pos;

    dmc_get_position_unit(CurrCardNo,Detector_W,&current_pos);//dd马达旋转轴
    axis.Detector_WPos = current_pos;
    return axis;
}


QString MotionCtrlCommand::getValueFromAdaptorIni(QString str)
{
    if(adaptorSharedPointer.isNull()) {
        return "";
    }
    return adaptorSharedPointer->value(str).toString();
}

void MotionCtrlCommand::setValue2AdaptorIni(QString str, QString value)
{
    if(adaptorSharedPointer.isNull()) {
        return;
    }
    adaptorSharedPointer->setValue(str,value);
}
void MotionCtrlCommand::pwmOutput(double frame)
{
    ushort enable,pwmNO;
    double fDuty,fFre;
    enable = 1;//使能
    pwmNO = 0;//通道0 为本地out2
    fDuty = 0.5;//占空比 50%
    fFre = frame;//输出频率 100000 HZ


    dmc_set_pwm_enable_extern(CurrCardNo,pwmNO,enable);


    dmc_set_pwm_output(CurrCardNo,pwmNO,fDuty,fFre);

    qDebug()<<"pwmOutput"<<frame;

}

bool MotionCtrlCommand::ptpMoveXY()
{

    QtConcurrent::run([=]{
        QString absSpeedStr;
        double vel;
        double tgtPos ;
        // if(!vecTableRowFlag)
        // {
        vecTableRow = axisformula.csvParsing();
        // }
        if(vecTableRow.empty())
            return false;

        QThread::msleep(1000);

        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed1");
        vel = absSpeedStr.toDouble();
        tgtPos = vecTableRow[0].AxisX.toFloat();
        qDebug()<<"readTableWidget"<<vecTableRow[0].AxisX<<" "<<tgtPos<<" "<<vecTableRow[0].AxisY;
        ptpMove(axisNo1,vel,tgtPos,1);


        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed2");
        vel = absSpeedStr.toDouble();
        tgtPos =vecTableRow[0].AxisY.toFloat();
        ptpMove(axisNo3,vel,tgtPos,1);


        while(dmc_check_done(CurrCardNo,axisNo1) == 0)
        {
            // qDebug()<<"axis1 00000000";
        }

        while(dmc_check_done(CurrCardNo,axisNo3) == 0)
        {
            // qDebug()<<"axis2  00000000";
        }
        // QThread::msleep(2000);
        qDebug()<<"1111111111111";
        emit sig_shoot();



        vecTableRow.pop_front();


    });

    return true;
}


void MotionCtrlCommand::slot_beamShootFinished()
{
    QString absSpeedStr;
    double vel;
    double tgtPos ;

    if(!vecStartFlag)
        return;

    if(vecTableRow.empty())
        return ;

    absSpeedStr = getValueFromIni("MotionCtrl/absSpeed1");
    vel = absSpeedStr.toDouble();
    tgtPos = vecTableRow[0].AxisX.toDouble();
    ptpMove(axisNo1,vel,tgtPos,1);


    absSpeedStr = getValueFromIni("MotionCtrl/absSpeed2");
    vel = absSpeedStr.toDouble();
    tgtPos =vecTableRow[0].AxisY.toDouble();
    ptpMove(axisNo3,vel,tgtPos,1);


    while(dmc_check_done(CurrCardNo,axisNo1) == 0)
    {

    }

    while(dmc_check_done(CurrCardNo,axisNo3) == 0)
    {

    }
    emit sig_shoot();

    vecTableRow.pop_front();
}

