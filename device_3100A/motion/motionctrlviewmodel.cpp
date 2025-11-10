#include "motionctrlviewmodel.h"
#include "LTDMC.h"
#include "device_3100A/xray/xraycontroller.h"
#include <QThread>
using namespace std;

MotionCtrlViewModel::MotionCtrlViewModel(QObject *parent)
    : QObject{parent}
{
    iniPath = QCoreApplication::applicationDirPath() +"/MotionCtrlConfig.ini";
    QFileInfo fileInfo(iniPath);
    if(fileInfo.isFile())
    {
        configSharedPointer.reset(new QSettings(iniPath,QSettings::IniFormat));
    }
    this->init();

    IoPortMap.insert("Door",0);
    IoPortMap.insert("RedLight",12);
    IoPortMap.insert("YellowLight",10);
    IoPortMap.insert("GreenLight",11);
    IoPortMap.insert("Alarm",13);
    IoPortMap.insert("XRay",14);
    // for(int i = 0;i<24;i++)
    // {
    //     if(i == 8)
    //     {
    //         dmc_write_outbit(0,i,0);//对输出口i 置低电平   0：低电平
    //         continue;
    //     }
    //     dmc_write_outbit(0,i,1);//对输出口i 置高电平    1：高电平
    // }
    m_pXrayController = new XrayController;
    m_pXrayThread = new QThread(this);
    m_pXrayController->moveToThread(m_pXrayThread);
    connect(m_pXrayThread, &QThread::finished, m_pXrayController, &QObject::deleteLater);
    // 2. ViewModel -> Controller 信号槽 (跨线程)
    //    (连接 ViewModel 发出的信号到 Controller 的槽)
    connect(this, &MotionCtrlViewModel::doInitializeXrayCom,
            m_pXrayController, &XrayController::initialize);
    connect(this, &MotionCtrlViewModel::doTurnXrayOn,
            m_pXrayController, &XrayController::turnXrayOn);

    // 3. 启动线程
    m_pXrayThread->start();

    // 4. 发送信号，让 Controller 在其新线程中开始初始化 COM
    emit doInitializeXrayCom();
}

QString MotionCtrlViewModel::loadDdSpeed(int index)
{
    if(index<0||index>13) {
        return "";
    }
    return getValueFromIni("MotionCtrl/ddSpeed" + QString::number(index + 1));
}

QString MotionCtrlViewModel::loadAbsSpeed(int index)
{
    if(index<0||index>13) {
        return "";
    }
    return getValueFromIni("MotionCtrl/absSpeed" + QString::number(index + 1));
}


void MotionCtrlViewModel::saveSpeed(int index, QString ddSpeed, QString absSpeed)
{
    if(index<0||index>13) {
        return;
    }
//    if(ddSpeed.toInt() <0) {
//        ddSpeed = "0";
//    }
//    if(absSpeed.toInt() <0) {
//        absSpeed = "0";
//    }
//    if(ddSpeed.toInt() >50000) {
//        ddSpeed = "50000";
//    }
//    if(absSpeed.toInt() >50000) {
//        absSpeed = "50000";
//    }
    setValue2Ini("MotionCtrl/ddSpeed" + QString::number(index + 1),ddSpeed);
    setValue2Ini("MotionCtrl/absSpeed" + QString::number(index + 1),absSpeed);
}

MotionCtrlViewModel::~MotionCtrlViewModel()
{
    qDebug()<<"~MotionCtrlViewModel";

    // mcManger->ReleaseIsRun();
    for(int i = 0;i<24;i++)
    {
        dmc_write_outbit(0,i,1);
    }
    m_pXrayThread->quit();
    m_pXrayThread->wait();
}
void MotionCtrlViewModel::onButtonXonClicked()
{
    // 发送信号给在 QThread 中运行的 XrayController
    emit doTurnXrayOn();
}
void MotionCtrlViewModel::onDataChanged(const QVariant &var)
{

    QVector<MotionCtrlData::MotionCtrlInfo> mcVec = var.value<QVector<MotionCtrlData::MotionCtrlInfo>>();

    int size = mcVec.size();
    for(int i =0 ;i < size;i++) {
        //使能状态
        bool status = mcVec[i].enable;
        {
            switch (i) {
            case 0:
                emit sig_enableChanged(0,status);
                break;
            case 1:
                emit sig_enableChanged(1,status);
                break;
            case 2:
                emit sig_enableChanged(2,status);
                break;
            case 3:
                emit sig_enableChanged(3,status);
                break;
            case 4:
                emit sig_enableChanged(4,status);
                break;
            case 5:
                emit sig_enableChanged(5,status);
                break;
            case 6:
                emit sig_enableChanged(6,status);
                break;
            case 7:
                emit sig_enableChanged(7,status);
                break;
            case 8:
                emit sig_enableChanged(8,status);
                break;
            case 9:
                emit sig_enableChanged(9,status);
                break;
            case 10:
                emit sig_enableChanged(10,status);
                break;
            case 11:
                emit sig_enableChanged(11,status);
                break;
            case 12:
                emit sig_enableChanged(12,status);
                break;
            default:
                break;
            }
        }
    }
}


void MotionCtrlViewModel::onBtn_openDoorClicked()
{
    if(!connected) return;
    // emit mcManger->sig_doorStatusChanged(true);

    short res = dmc_write_outbit(0,IoPortMap["Door"],1);
    if( !res )
    {
        disconnect(doorStatusBtn,&QPushButton::clicked,this,&MotionCtrlViewModel::onBtn_openDoorClicked);
        connect(doorStatusBtn,&QPushButton::clicked,this,&MotionCtrlViewModel::onBtn_closeDoorClicked);
        doorStatusBtn->setText(u8"关门");
    }
}

void MotionCtrlViewModel::onBtn_closeDoorClicked()
{
    if(!connected) return;

    // emit mcManger->sig_doorStatusChanged(false);
    short res = dmc_write_outbit(0,IoPortMap["Door"],0);

    //mcManger->mcCommand->rayClose();



    if(!res)
    {
        disconnect(doorStatusBtn,&QPushButton::clicked,this,&MotionCtrlViewModel::onBtn_closeDoorClicked);
        connect(doorStatusBtn,&QPushButton::clicked,this,&MotionCtrlViewModel::onBtn_openDoorClicked);
        doorStatusBtn->setText(u8"开门");
    }
}

void MotionCtrlViewModel::onBtn_ClearAxisErrcode()
{
    if(!connected) return;

    mcManger->clearAxisErrcode();
}

void MotionCtrlViewModel::onBtn_axisLeftPressed(int axis)
{
    log_(QString("ACTION: Jog Left Pressed for axis %1").arg(axis)); // <-- 新增日志
    if(!connected) return;
    log_(QString("ACTION: Jog Left Pressed for axis %1").arg(axis)); // <-- 新增日志
    QMap<int, QPair<bool, bool> >  pos_limits = mcManger->getCtrlCommand()->getInstance()->getPosLimit();

    // if(pos_limits[axis].first||pos_limits[axis].second)
    // {
    //     emit sig_posLimit();
    //     return ;
    // }
    QString value1;
    double vel;
    switch (axis) {
    case 0:
        value1 = getValueFromIni("MotionCtrl/ddSpeed1");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo1,vel,0);
        break;
    case 1:
        value1 = getValueFromIni("MotionCtrl/ddSpeed2");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo2,vel,0);
        break;
    case 2:
        value1 = getValueFromIni("MotionCtrl/ddSpeed3");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo3,vel,0);
        break;
    case 3:
        value1 = getValueFromIni("MotionCtrl/ddSpeed4");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo4,vel,0);
        break;
    case 4:
        value1 = getValueFromIni("MotionCtrl/ddSpeed5");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo5,vel,0);
        break;
    case 5:
        value1 = getValueFromIni("MotionCtrl/ddSpeed6");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo6,vel,0);
        break;
    case 6:
        value1 = getValueFromIni("MotionCtrl/ddSpeed7");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo7,vel,0);
        break;
    case 7:
        value1 = getValueFromIni("MotionCtrl/ddSpeed8");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo8,vel,0);
        break;
    case 8:
        value1 = getValueFromIni("MotionCtrl/ddSpeed9");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo9,vel,0);
        break;
    case 9:
        value1 = getValueFromIni("MotionCtrl/ddSpeed10");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo10,vel,0);
        break;
    case 10:
        value1 = getValueFromIni("MotionCtrl/ddSpeed11");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo11,vel,0);
        break;
    case 11:
        value1 = getValueFromIni("MotionCtrl/ddSpeed12");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo12,vel,0);
        break;
    case 12:
        value1 = getValueFromIni("MotionCtrl/ddSpeed13");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo13,vel,0);
        break;
    default:
        break;
    }
}

void MotionCtrlViewModel::onBtn_axisStopClicked(int axis)
{
    if(!connected) return;

    switch (axis) {
    case 0:
        mcManger->axisStop(mcManger->axisNo1,1);
        break;
    case 1:
        mcManger->axisStop(mcManger->axisNo2,1);
        break;
    case 2:
        mcManger->axisStop(mcManger->axisNo3,1);
        break;
    case 3:
        mcManger->axisStop(mcManger->axisNo4,1);
        break;
    case 4:
        mcManger->axisStop(mcManger->axisNo5,1);
        break;
    case 5:
        mcManger->axisStop(mcManger->axisNo6,1);
        break;
    case 6:
        mcManger->axisStop(mcManger->axisNo7,1);
        break;
    case 7:
        mcManger->axisStop(mcManger->axisNo8,1);
        break;
    case 8:
        mcManger->axisStop(mcManger->axisNo9,1);
        break;
    case 9:
        mcManger->axisStop(mcManger->axisNo10,1);
        break;
    case 10:
        mcManger->axisStop(mcManger->axisNo11,1);
        break;
    case 11:
        mcManger->axisStop(mcManger->axisNo12,1);
        break;
    case 12:
        mcManger->axisStop(mcManger->axisNo13,1);
        break;
    default:
        break;
    }
}

void MotionCtrlViewModel::onBtn_axisRightPressed(int axis)
{
    if(!connected) return;
    QString value1;
    double vel;

    QMap<int, QPair<bool, bool> >  pos_limits = mcManger->getCtrlCommand()->getInstance()->getPosLimit();

    // if(pos_limits[axis].first||pos_limits[axis].second)
    // {
    //     emit sig_posLimit();
    //     return ;
    // }

    switch (axis) {
    case 0:
        value1 = getValueFromIni("MotionCtrl/ddSpeed1");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo1,vel,1);
        break;
    case 1:
        value1 = getValueFromIni("MotionCtrl/ddSpeed2");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo2,vel,1);
        break;
    case 2:
        value1 = getValueFromIni("MotionCtrl/ddSpeed3");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo3,vel,1);
        break;
    case 3:
        value1 = getValueFromIni("MotionCtrl/ddSpeed4");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo4,vel,1);
        break;
    case 4:
        value1 = getValueFromIni("MotionCtrl/ddSpeed5");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo5,vel,1);
        break;
    case 5:
        value1 = getValueFromIni("MotionCtrl/ddSpeed6");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo6,vel,1);
        break;
    case 6:
        value1 = getValueFromIni("MotionCtrl/ddSpeed7");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo7,vel,1);
        break;
    case 7:
        value1 = getValueFromIni("MotionCtrl/ddSpeed8");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo8,vel,1);
        break;
    case 8:
        value1 = getValueFromIni("MotionCtrl/ddSpeed9");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo9,vel,1);
        break;
    case 9:
        value1 = getValueFromIni("MotionCtrl/ddSpeed10");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo10,vel,1);
        break;
    case 10:
        value1 = getValueFromIni("MotionCtrl/ddSpeed11");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo11,vel,1);
        break;
    case 11:
        value1 = getValueFromIni("MotionCtrl/ddSpeed12");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo12,vel,1);
        break;
    case 12:
        value1 = getValueFromIni("MotionCtrl/ddSpeed13");
        vel = value1.toDouble();
        mcManger->jogMove(mcManger->axisNo13,vel,1);
        break;
    default:
        break;
    }
}

void MotionCtrlViewModel::onBtn_axisStartClicked(int axis, QString value)
{
    if(!connected) return;

    QString absSpeedStr;
    double vel;
    double tgtPos ;
    // QMap<int, QPair<bool, bool> >  pos_limits = mcManger->getCtrlCommand()->getInstance()->getPosLimit();

    // if(pos_limits[axis].first||pos_limits[axis].second)
    // {
    //     emit sig_posLimit();
    //     return ;
    // }
    switch (axis) {
    case 0:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed1");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo1,vel,tgtPos,1);
        break;
    case 1:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed2");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo2,vel,tgtPos,1);
        break;
    case 2:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed3");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo3,vel,tgtPos,1);
        break;
    case 3:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed4");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo4,vel,tgtPos,1);
        break;
    case 4:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed5");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo5,vel,tgtPos,1);
        break;
    case 5:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed6");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo6,vel,tgtPos,1);
        break;
    case 6:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed7");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo7,vel,tgtPos,1);
        break;
    case 7:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed8");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo8,vel,tgtPos,1);
        break;
    case 8:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed9");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo9,vel,tgtPos,1);
        break;
    case 9:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed10");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo10,vel,tgtPos,1);
        break;
    case 10:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed11");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo11,vel,tgtPos,1);
        break;
    case 11:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed12");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo12,vel,tgtPos,1);
        break;
    case 12:
        absSpeedStr = getValueFromIni("MotionCtrl/absSpeed13");
        vel = absSpeedStr.toDouble();
        tgtPos = value.toDouble();
        mcManger->ptpMove(mcManger->axisNo13,vel,tgtPos,1);
        break;

    default:
        break;
    }
}

void MotionCtrlViewModel::onBtn_axisResetPosClicked(int axis)
{
    // if(!connected) return;

    mcManger->axHomingMove(axis);

}

void MotionCtrlViewModel::onBtn_axisEnableClicked(int axis, bool status)
{
    if(!connected) return;

    switch (axis) {
    case 0:
        if(status) {
            mcManger->axEnable(mcManger->axisNo1);
        } else {
            mcManger->axDisenable(mcManger->axisNo1);
        }
        break;
    case 1:
        if(status) {
            mcManger->axEnable(mcManger->axisNo2);
        } else {
            mcManger->axDisenable(mcManger->axisNo2);
        }
        break;
    case 2:
        if(status) {
            mcManger->axEnable(mcManger->axisNo3);
        } else {
            mcManger->axDisenable(mcManger->axisNo3);
        }
        break;
    case 3:
        if(status) {
            mcManger->axEnable(mcManger->axisNo4);
        } else {
            mcManger->axDisenable(mcManger->axisNo4);
        }
        break;
    case 4:
        if(status) {
            mcManger->axEnable(mcManger->axisNo5);
        } else {
            mcManger->axDisenable(mcManger->axisNo5);
        }
        break;
    case 5:
        if(status) {
            mcManger->axEnable(mcManger->axisNo6);
        } else {
            mcManger->axDisenable(mcManger->axisNo6);
        }
        break;
    case 6:
        if(status) {
            mcManger->axEnable(mcManger->axisNo7);
        } else {
            mcManger->axDisenable(mcManger->axisNo7);
        }
        break;
    case 7:
        if(status) {
            mcManger->axEnable(mcManger->axisNo8);
        } else {
            mcManger->axDisenable(mcManger->axisNo8);
        }
        break;
    case 8:
        if(status) {
            mcManger->axEnable(mcManger->axisNo9);
        } else {
            mcManger->axDisenable(mcManger->axisNo9);
        }
        break;
    case 9:
        if(status) {
            mcManger->axEnable(mcManger->axisNo10);
        } else {
            mcManger->axDisenable(mcManger->axisNo10);
        }
        break;
    case 10:
        if(status) {
            mcManger->axEnable(mcManger->axisNo11);
        } else {
            mcManger->axDisenable(mcManger->axisNo11);
        }
        break;
    case 11:
        if(status) {
            mcManger->axEnable(mcManger->axisNo12);
        } else {
            mcManger->axDisenable(mcManger->axisNo12);
        }
        break;
    case 12:
        if(status) {
            mcManger->axEnable(mcManger->axisNo13);
        } else {
            mcManger->axDisenable(mcManger->axisNo13);
        }
        break;
    default:
        break;
    }
}

void MotionCtrlViewModel::onBtn_softResetClicked()
{
    if(!connected) return;
    mcManger->softReset();
}

void MotionCtrlViewModel::onBtn_boardResetClicked()
{
    if(!connected) return;
    mcManger->boardReset();
}

void MotionCtrlViewModel::init()
{
    mcManger = MotionCtrlManager::getInstance();
    connect(mcManger,&MotionCtrlManager::sig_connect,this,[=](bool status){
        connected = status;
//        qDebug() <<connected;
    });

    connect(mcManger,&MotionCtrlManager::sig_resetTime,this,&MotionCtrlViewModel::sig_resetTime);

    connect(mcManger,&MotionCtrlManager::sig_dataChanged,this,&MotionCtrlViewModel::onDataChanged);

    connect(this,&MotionCtrlViewModel::sig_beamContinuousScanning,mcManger,&MotionCtrlManager::sig_beamContinuousScanning);


    rayX_nLimit = getValueFromIni("ImagingAnchorParam/rayX_nLimit").toDouble();
    rayX_pLimit = getValueFromIni("ImagingAnchorParam/rayX_pLimit").toDouble();
    probeX_nLimit = getValueFromIni("ImagingAnchorParam/probeX_nLimit").toDouble();
    probeX_pLimit = getValueFromIni("ImagingAnchorParam/probeX_pLimit").toDouble();
    loaderY_nLimit = getValueFromIni("ImagingAnchorParam/loaderY_nLimit").toDouble();
    loaderY_pLimit = getValueFromIni("ImagingAnchorParam/loaderY_pLimit").toDouble();
    loaderZ_nLimit = getValueFromIni("ImagingAnchorParam/loaderZ_nLimit").toDouble();
    loaderZ_nLimit = getValueFromIni("ImagingAnchorParam/loaderZ_pLimit").toDouble();
}

QString MotionCtrlViewModel::getValueFromIni(QString str)
{
    if(configSharedPointer.isNull()) {
        return "";
    }
    return configSharedPointer->value(str).toString();
}

void MotionCtrlViewModel::setValue2Ini(QString str, QString value)
{
    if(configSharedPointer.isNull()) {
        return;
    }
    configSharedPointer->setValue(str,value);
}

void MotionCtrlViewModel::beamContinuousScanning()
{
    // double speed;
    QString value1 = getValueFromIni("MotionCtrl/absSpeed7");
    double speed = value1.toDouble();

    emit sig_beamContinuousScanning(speed,500);
}

void MotionCtrlViewModel::onBtn_axisStartXYClicked( )
{

    if(!connected) return;


    mcManger->ptpMoveXY();


}
