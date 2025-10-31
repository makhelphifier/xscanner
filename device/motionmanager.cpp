#include "motionmanager.h"

QMutex MotionManager:: m_Mutex{};
MotionManager *MotionManager::sm_pInstance = nullptr;

MotionManager *MotionManager::getInstance()
{
    if (sm_pInstance == nullptr) {
        m_Mutex.lock();
        if (sm_pInstance == nullptr) {
            sm_pInstance = new MotionManager();
        }
    }
    return sm_pInstance;
}

void MotionManager::boardReset()
{
    emit sig_boardReset();
}

void MotionManager::softReset()
{
    emit sig_softReset();
}

void MotionManager::ptpMove(ushort axisNo, double vel, double dist, ushort pos_mode)
{
    emit sig_ptpMove(axisNo, vel, dist, pos_mode);
}

void MotionManager::jogMove(ushort axisNo, double vel, ushort dir)
{
    emit sig_jogMove(axisNo, vel, dir);
}

void MotionManager::axHomingMove(ushort axisNo)
{
    emit sig_axHomingMove(axisNo);
}

double MotionManager::getAnnulusWPos()
{
    return mcCommand->getAnnulusWPos();
}

void MotionManager::autoReset()
{
    emit sig_autoReset();
}

void MotionManager::claming_btn_clicked()
{
    emit sig_claming_loose();
}

void MotionManager::mau_btn_clicked()
{
    emit sig_mau();
}

void MotionManager::auto_btn_clicked()
{
    emit sig_auto();
}


void MotionManager::axEnable(ushort axisNo)
{
    emit sig_axEnable(axisNo);
}

void MotionManager::axDisenable(ushort axisNo)
{
    emit sig_axDisenable(axisNo);
}

void MotionManager::axisStop(ushort axisNo, ushort stopMode)
{
    emit sig_axisStop(axisNo, stopMode);
}

void MotionManager::emgStop()
{
    emit sig_emgStop();
}

void MotionManager::clearAxisErrcode()
{
    emit sig_ClearAxisErrcode();
}

void MotionManager::writeOutbit(ushort bitNo, ushort out)
{
    emit sig_writeOutbit(bitNo,out);
}

void MotionManager::setDoStatus(ushort bitNo)
{
    emit sig_setDoStatus(bitNo);
}

const MotionCtrlCommand *MotionManager::getCtrlCommand() const
{
    return mcCommand;
}

void MotionManager::ReleaseIsRun()
{
    if(releaseFlag)
    {
        releaseFlag = false;
        qDebug()<<"MotionCtrlManager ~ReleaseIsRun";
        if (m_thread.isRunning()) {
            // 停止定时器任务
            // QMetaObject::invokeMethod(mcCommand, "slot_stopTimerTask", Qt::BlockingQueuedConnection);
            mcCommand->slot_stopTimerTask();
            // 退出线程
            m_thread.quit();

            // 等待线程正常退出
            if (!m_thread.wait(1000)) {
                qWarning() << "Thread not responding, terminating...";
                m_thread.terminate();
                m_thread.wait();

            }
        }
    }
}

void MotionManager::beamStepShootFinished()
{
    emit sig_beamStepShootFinished();
}

//3组扫描完成
void MotionManager::beamShootFinished()
{
    emit sig_beamShootFinished();
}

//扫描停止按钮
void MotionManager::beamContiShootFinished()
{
    emit sig_beamContiShootFinished();
}

//旋转W 旋转
void MotionManager::upper_material_W_Pos(int index,double speed, double startAngle, double stopAngle, float stepLength)
{
    emit sig_upper_material_W_Pos(index,speed,startAngle,stopAngle,stepLength);
}

void MotionManager::SlipRingBackToStartingPos(double startPos)
{
    emit sig_SlipRingBackToStartingPos(startPos);
}

void MotionManager::beamContinuousScanning(double degSpeed,double start,double endRange,float step)
{
    emit sig_beamContinuousScanning(degSpeed,start,endRange,step);
}

void MotionManager::beamStepScanning(int degRange, int step)
{
    emit sig_beamStepScanning(degRange,step);
}

void MotionManager::beamEmsStop()
{
    emit sig_beamEmsStop();
}

void MotionManager::allaxisHoming()
{
    emit sig_allaxisHoming();
}

MotionManager::MotionManager(QObject *parent)
    : QObject{parent}
{
    qDebug() << "this main thread " << QThread::currentThreadId();
    mcCommand = MotionCtrlCommand::getInstance();
    mcCommand->moveToThread(&m_thread);
    connect(&m_thread,&QThread::started,mcCommand,&MotionCtrlCommand::slot_startTimerTask);
    connect(&m_thread,&QThread::finished, mcCommand, &MotionCtrlCommand::slot_stopTimerTask, Qt::DirectConnection);
    // connect(&m_thread,&QThread::finished, mcCommand, &MotionCtrlCommand::deleteLater, Qt::DirectConnection);
    connections();

    m_thread.start();
}

MotionManager::~MotionManager()
{
    qDebug()<<"~MotionCtrlManager";
    // m_thread.quit();
    // m_thread.wait();
    // m_thread.deleteLater();

}

void MotionManager::connections()
{
    connect(mcCommand, &MotionCtrlCommand::sig_resetTime, this, &MotionManager::sig_resetTime);

    connect(mcCommand, &MotionCtrlCommand::sig_connect, this, &MotionManager::sig_connect);

    connect(mcCommand, &MotionCtrlCommand::sig_dataChanged, this, &MotionManager::sig_dataChanged);

    connect(mcCommand, &MotionCtrlCommand::sig_DIStatusdataChanged, this, &MotionManager::sig_DIStatusdataChanged);
    connect(mcCommand, &MotionCtrlCommand::sig_allowBeamShooting, this, &MotionManager::sig_allowBeamShooting);

    connect(mcCommand, &MotionCtrlCommand::sig_motionMsg, this, &MotionManager::sig_motionMsg);

    connect(this, &MotionManager::sig_boardReset, mcCommand, &MotionCtrlCommand::boardReset);

    connect(this, &MotionManager::sig_softReset, mcCommand, &MotionCtrlCommand::softReset);

    connect(this, &MotionManager::sig_ptpMove, mcCommand, &MotionCtrlCommand::ptpMove);

    connect(this, &MotionManager::sig_jogMove, mcCommand, &MotionCtrlCommand::jogMove);

    connect(this, &MotionManager::sig_axHomingMove, mcCommand, &MotionCtrlCommand::axHomingMove);

    connect(this, &MotionManager::sig_axEnable, mcCommand, &MotionCtrlCommand::axEnable);

    connect(this, &MotionManager::sig_axDisenable, mcCommand, &MotionCtrlCommand::axDisenable);

    connect(this, &MotionManager::sig_axisStop, mcCommand, &MotionCtrlCommand::axisStop);

    connect(this, &MotionManager::sig_emgStop, mcCommand, &MotionCtrlCommand::emgStop);

    connect(this, &MotionManager::sig_ClearAxisErrcode, mcCommand, &MotionCtrlCommand::nmcClearAxisErrcode);

    connect(this, &MotionManager::sig_setDoStatus, mcCommand, &MotionCtrlCommand::setDoStatus);

    connect(this, &MotionManager::sig_writeOutbit, mcCommand, &MotionCtrlCommand::writeOutbit);

    // connect(this, &MotionCtrlManager::sig_beamContinuousScanning, mcCommand, &MotionCtrlCommand::beamContinuousScanning);

    // connect(this, &MotionCtrlManager::sig_beamContinuousScanning, mcCommand, &MotionCtrlCommand::HcmpBeamContinuousScanning);


    connect(this, &MotionManager::sig_releaseIsRun, mcCommand, &MotionCtrlCommand::slot_releaseIsRun);
    //锥束持续扫描信号
    // connect(mcCommand,&MotionCtrlCommand::sig_beamContiScanningStart,this,&MotionCtrlManager::sig_beamContiScanningStart);
    connect(mcCommand,&MotionCtrlCommand::sig_beamContiScanningEnd,this,&MotionManager::sig_beamContiScanningEnd);
    connect(mcCommand,&MotionCtrlCommand::sig_allowBeamShooting,this,&MotionManager::sig_allowBeamShooting);
    //锥束步进扫描信号
    connect(mcCommand,&MotionCtrlCommand::sig_beamStepScanningStart,this,&MotionManager::sig_beamStepScanningStart);
    connect(mcCommand,&MotionCtrlCommand::sig_beamStepScanningEnd,this,&MotionManager::sig_beamStepScanningEnd);

    connect(mcCommand,&MotionCtrlCommand::sig_ArrivalDetectionPos,this,&MotionManager::sig_ArrivalDetectionPos);

    //滑环运动完成
    connect(mcCommand,&MotionCtrlCommand::sig_TheSlipRingMovemenIsCompleted,this,&MotionManager::sig_TheSlipRingMovemenIsCompleted);


    connect(this,&MotionManager::sig_beamContinuousScanning,mcCommand,&MotionCtrlCommand::beamContinuousScanning);

    connect(this,&MotionManager::sig_beamStepScanning,mcCommand,&MotionCtrlCommand::beamStepScanning);

    connect(this,&MotionManager::sig_beamContiShootFinished,mcCommand,&MotionCtrlCommand::slot_beamContiShootFinished);

    connect(this,&MotionManager::sig_beamStepShootFinished,mcCommand,&MotionCtrlCommand::slot_beamStepShootFinished);

    connect(this,&MotionManager::sig_beamEmsStop,mcCommand,&MotionCtrlCommand::slot_beamEmsStop);

    connect(this,&MotionManager::sig_allaxisHoming,mcCommand,&MotionCtrlCommand::allaxisHoming);

    connect(this,&MotionManager::sig_SlipRingBackToStartingPos,mcCommand,&MotionCtrlCommand::SlipRingBackToStartingPos);

}

