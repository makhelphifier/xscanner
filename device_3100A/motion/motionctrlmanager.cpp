#include "motionctrlmanager.h"

QMutex MotionCtrlManager:: m_Mutex{};
MotionCtrlManager *MotionCtrlManager::sm_pInstance = nullptr;

MotionCtrlManager *MotionCtrlManager::getInstance()
{
    // QMutexLocker mutexLocker(&m_Mutex);
    // static MotionCtrlManager instance;
    // return &instance;

    if (sm_pInstance == nullptr) {
        m_Mutex.lock();
        if (sm_pInstance == nullptr) {
            sm_pInstance = new MotionCtrlManager();
        }
    }
    return sm_pInstance;
}

void MotionCtrlManager::boardReset()
{
    emit sig_boardReset();
}

void MotionCtrlManager::softReset()
{
    emit sig_softReset();
}

void MotionCtrlManager::ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode)
{
    emit sig_ptpMove(axisNo, vel, dist, pos_mode);
}

void MotionCtrlManager::jogMove(ushort axisNo, double vel, ushort dir)
{
    emit sig_jogMove(axisNo, vel, dir);
}

void MotionCtrlManager::axHomingMove(ushort axisNo)
{
    emit sig_axHomingMove(axisNo);
}

void MotionCtrlManager::axEnable(ushort axisNo)
{
    emit sig_axEnable(axisNo);
}

void MotionCtrlManager::axDisenable(ushort axisNo)
{
    emit sig_axDisenable(axisNo);
}

void MotionCtrlManager::axisStop(ushort axisNo, ushort stopMode)
{
    emit sig_axisStop(axisNo, stopMode);
}

void MotionCtrlManager::emgStop()
{
    emit sig_emgStop();
}

void MotionCtrlManager::clearAxisErrcode()
{
    emit sig_ClearAxisErrcode();
}

const MotionCtrlCommand *MotionCtrlManager::getCtrlCommand() const
{
    return mcCommand;
}

void MotionCtrlManager::ReleaseIsRun()
{
    emit sig_releaseIsRun();
    qDebug()<<"MotionCtrlManager ~ReleaseIsRun";
    m_thread.quit();
    m_thread.wait();
    m_thread.deleteLater();
}

void MotionCtrlManager::beamStepShootFinished()
{
    emit sig_beamStepShootFinished();
}

void MotionCtrlManager::beamShootFinished()
{
    emit sig_beamShootFinished();
}

void MotionCtrlManager::beamContiShootFinished()
{
    emit sig_beamContiShootFinished();
}

void MotionCtrlManager::beamContinuousScanning(float speed,double angleRange)
{
    emit sig_beamContinuousScanning(speed,angleRange);
}

void MotionCtrlManager::beamStepScanning(float degRange, double step)
{
    emit sig_beamStepScanning(degRange,step);
}

void MotionCtrlManager::beamEmsStop()
{
    emit sig_beamEmsStop();
}

void MotionCtrlManager::batteryClampRotates(float angle)
{

    emit sig_batteryClampRotates(angle);
}

MotionCtrlManager::MotionCtrlManager(QObject *parent)
    : QObject{parent}
{
    qDebug() << "this main thread " << QThread::currentThreadId();
    mcCommand = MotionCtrlCommand::getInstance();
    mcCommand->moveToThread(&m_thread);
    connect(&m_thread,&QThread::started,mcCommand,&MotionCtrlCommand::slot_startTimerTask);
    connect(&m_thread,&QThread::finished, mcCommand, &MotionCtrlCommand::slot_stopTimerTask, Qt::DirectConnection);
    connect(&m_thread,&QThread::finished, mcCommand, &MotionCtrlCommand::deleteLater, Qt::DirectConnection);
    connections();
    axisNo1 = mcCommand->axisNo1;
    axisNo2 = mcCommand->axisNo2;
    axisNo3 = mcCommand->axisNo3;
    axisNo4 = mcCommand->axisNo4;
    axisNo5 = mcCommand->axisNo5;
    axisNo6 = mcCommand->axisNo6;
    axisNo7 = mcCommand->axisNo7;
    axisNo8 = mcCommand->axisNo8;
    axisNo9 = mcCommand->axisNo9;
    axisNo10 = mcCommand->axisNo10;
    axisNo11 = mcCommand->axisNo11;
    axisNo12 = mcCommand->axisNo12;
    axisNo13 = mcCommand->axisNo13;
    axisNo14 = mcCommand->axisNo14;
    axisNo15 = mcCommand->axisNo15;


    m_thread.start();
}


void MotionCtrlManager::slotDDAxisPos(QVector<DDStatus> val)
{

    if(DDposFlagsave && DDposFlag)
    {
        qDebug()<<"DDposFlagsave:"<<DDposFlagsave<<" "<<DDposFlag;
        DDposFlagsave = false;
        DDposFlag = false;
        QString filename = fileDDPos+"/axispos.txt";
        qDebug()<<"filename:"<<filename<<"  vecDDPos:"<<val.size();

        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            QTextStream stream(&file);

            for(int i = 0; i < val.size();i++)
            {
                // 获取当前时间（精确到秒）

                // 格式化写入
                stream << i << " \t" << val[i].timer << " \t" << val[i].objectiveTable_X1Pos << " \t"
                       << val[i].objectiveTable_Y1Pos << " \t" << val[i].Detector_RPos << " \t" << val[i].Detector_WPos<< "\n";

            }

            file.close();

            qDebug() << u8"数据保存成功 " ;
        } else {
            qWarning() << u8"无法打开文件：" << filename;
        }

        vecDDstatus.clear();
    }

}


MotionCtrlManager::~MotionCtrlManager()
{
    qDebug()<<"~MotionCtrlManager";
    delete sm_pInstance ;
    sm_pInstance = nullptr;
    m_thread.quit();
    m_thread.wait();
    m_thread.deleteLater();

}

void MotionCtrlManager::connections()
{
    connect(mcCommand, &MotionCtrlCommand::sig_resetTime, this, &MotionCtrlManager::sig_resetTime);

    connect(mcCommand, &MotionCtrlCommand::sig_connect, this, &MotionCtrlManager::sig_connect);

    connect(mcCommand, &MotionCtrlCommand::sig_dataChanged, this, &MotionCtrlManager::sig_dataChanged);

    connect(mcCommand, &MotionCtrlCommand::sig_allowBeamShooting, this, &MotionCtrlManager::sig_allowBeamShooting);

    connect(this, &MotionCtrlManager::sig_boardReset, mcCommand, &MotionCtrlCommand::boardReset);

    connect(this, &MotionCtrlManager::sig_softReset, mcCommand, &MotionCtrlCommand::softReset);

    connect(this, &MotionCtrlManager::sig_ptpMove, mcCommand, &MotionCtrlCommand::ptpMove);

    connect(this, &MotionCtrlManager::sig_ptpMoveXY, mcCommand, &MotionCtrlCommand::ptpMoveXY);


    connect(this, &MotionCtrlManager::sig_jogMove, mcCommand, &MotionCtrlCommand::jogMove);

    connect(this, &MotionCtrlManager::sig_axHomingMove, mcCommand, &MotionCtrlCommand::axHomingMove);

    connect(this, &MotionCtrlManager::sig_axEnable, mcCommand, &MotionCtrlCommand::axEnable);

    connect(this, &MotionCtrlManager::sig_axDisenable, mcCommand, &MotionCtrlCommand::axDisenable);

    connect(this, &MotionCtrlManager::sig_axisStop, mcCommand, &MotionCtrlCommand::axisStop);

    connect(this, &MotionCtrlManager::sig_emgStop, mcCommand, &MotionCtrlCommand::emgStop);

    connect(this, &MotionCtrlManager::sig_ClearAxisErrcode, mcCommand, &MotionCtrlCommand::nmcClearAxisErrcode);

    connect(mcCommand, &MotionCtrlCommand::sig_switchAngleComplete, this, [this](){
        emit sig_didMoveToAngle();
    });

    connect(mcCommand, &MotionCtrlCommand::sig_shoot, this, &MotionCtrlManager::sig_shoot);

    connect(this,&MotionCtrlManager::sig_beamShootFinished,mcCommand,&MotionCtrlCommand::slot_beamShootFinished);

    // connect(this, &MotionCtrlManager::sig_beamContinuousScanning, mcCommand, &MotionCtrlCommand::beamContinuousScanning);

    // connect(this, &MotionCtrlManager::sig_beamContinuousScanning, mcCommand, &MotionCtrlCommand::HcmpBeamContinuousScanning);


    connect(this, &MotionCtrlManager::sig_releaseIsRun, mcCommand, &MotionCtrlCommand::slot_releaseIsRun);
    //锥束持续扫描信号
    // connect(mcCommand,&MotionCtrlCommand::sig_beamContiScanningStart,this,&MotionCtrlManager::sig_beamContiScanningStart);
    connect(mcCommand,&MotionCtrlCommand::sig_beamContiScanningEnd,this,&MotionCtrlManager::sig_beamContiScanningEnd);
    connect(mcCommand,&MotionCtrlCommand::sig_allowBeamShooting,this,&MotionCtrlManager::sig_allowBeamShooting);
    //锥束步进扫描信号
    connect(mcCommand,&MotionCtrlCommand::sig_beamStepScanningStart,this,&MotionCtrlManager::sig_beamStepScanningStart);
    connect(mcCommand,&MotionCtrlCommand::sig_beamStepScanningEnd,this,&MotionCtrlManager::sig_beamStepScanningEnd);

    connect(this,&MotionCtrlManager::sig_beamContinuousScanning,mcCommand,&MotionCtrlCommand::beamContinuousScanning);

    // connect(this,&MotionCtrlManager::sig_beamStepScanning,mcCommand,&MotionCtrlCommand::beamStepScanning);
    // connect(this,&MotionCtrlManager::sig_beamStepShootFinished,mcCommand,&MotionCtrlCommand::slot_beamStepShootFinished);

    // connect(this,&MotionCtrlManager::sig_beamEmsStop,mcCommand,&MotionCtrlCommand::slot_beamEmsStop);
    //============================
    connect(this,&MotionCtrlManager::sig_beamStepScanning,mcCommand,&MotionCtrlCommand::beamStepScanning3100A);

    connect(this,&MotionCtrlManager::sig_beamStepShootFinished,mcCommand,&MotionCtrlCommand::beamStepScanningfinishShooting);

    connect(this,&MotionCtrlManager::sig_beamEmsStop,mcCommand,&MotionCtrlCommand::slot_beamEmsStop3100A);
    //================================
    // connect(this,&MotionCtrlManager::sig_spiralScanningMove,mcCommand,&MotionCtrlCommand::spiralScanningMove);

    connect(this,&MotionCtrlManager::sig_beamContiShootFinished,mcCommand,&MotionCtrlCommand::slot_beamContiShootFinished);

    // connect(this,&MotionCtrlManager::sig_beamStepShootFinished,mcCommand,&MotionCtrlCommand::slot_beamStepShootFinished);


    connect(this,&MotionCtrlManager::sig_batteryClampRotates,mcCommand,&MotionCtrlCommand::slot_batteryClampRotates);


}

void MotionCtrlManager::ptpMoveXY()
{
    emit sig_ptpMoveXY();
}
