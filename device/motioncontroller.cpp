#include "motioncontroller.h"
#include "motionconfig.h"
#include "../third_party/device/motor/include/LTDMC.h"
using namespace std;

MotionController::MotionController(QObject *parent)
    : QObject{parent}
{
    this->init();
}

MotionController::~MotionController()
{
    if(m_modelFlag)
    {
        m_modelFlag = false;
        m_mManger->ReleaseIsRun();
        if(m_mManger)
        {
            m_mManger->deleteLater();
            m_mManger = nullptr;
        }
    }

}

void MotionController::onBtn_ClearAxisErrcode()
{
    if(!m_connected) return;

    m_mManger->clearAxisErrcode();
}

void MotionController::onBtn_emgStop()
{
    m_mManger->emgStop();
}

void MotionController::onBtn_axisLeftPressed(int axis)
{
    if(!m_connected) return;

    double vel;
    vel =  MotionConfig::lastAxisSpeed[axis];
    m_mManger->jogMove(axis,vel,0);
}

void MotionController::onBtn_axisStopClicked(int axis)
{
    if(!m_connected) return;

    if(axis == 255)
    {
        for(int i = 0;i<5;i++)
        {
            m_mManger->axisStop(i,1);
        }
        //axDisenable
        m_mManger->axDisenable(255);//断使能

        QTimer::singleShot(2000, this, [this](){
            m_mManger->axEnable(255);//断使能
        });
    }else
    {
        m_mManger->axisStop(axis,1);
    }
}

void MotionController::onBtn_axisRightPressed(int axis)
{
    if(!m_connected) return;
    double vel;
    vel = MotionConfig::lastAxisSpeed[axis];
    qDebug()<<" MotionConfig::lastAxisSpeed[axis]:"<< MotionConfig::lastAxisSpeed[axis];
    m_mManger->jogMove(axis,vel,1);
}

void MotionController::onBtn_axisStartClicked(int axis, QString value)
{
    if(!m_connected) return;

    double vel;
    double tgtPos ;
    vel = MotionConfig::lastAxisSpeed[axis];
    tgtPos = value.toDouble();
    m_mManger->ptpMove(axis,vel,tgtPos,1);
}

void MotionController::onBtn_axisResetPosClicked(int axis)
{
    if(!m_connected) return;

    m_mManger->axHomingMove(axis);
}

void MotionController::onBtn_axisEnableClicked(int axis, bool status)
{
    if(!m_connected) return;

    qDebug()<<"axis enable clicked axis:"<<axis<<"  status:"<<status;
    if(status) {
        m_mManger->axEnable(axis);
    } else {
        m_mManger->axDisenable(axis);
    }
}

void MotionController::allaxisHoming()
{
    m_mManger->allaxisHoming();
}

void MotionController::setDoStatus(ushort bitNo)
{
    m_mManger->setDoStatus(bitNo);
}

void MotionController::writeOutbit(ushort bitNo, ushort out)
{
    m_mManger->writeOutbit(bitNo,out);
}

void MotionController::onBtn_softResetClicked()
{
    if(!m_connected)
        return;

    m_mManger->softReset();
}

void MotionController::onBtn_boardResetClicked()
{
    if(!m_connected)
        return;

    m_mManger->boardReset();
}

void MotionController::init()
{
    m_mManger = MotionManager::getInstance();
    connect(m_mManger,&MotionManager::sig_connect,this,[=](bool status){

        m_connected = status;
        emit sig_motionConnect(m_connected);
    });

    connect(m_mManger,&MotionManager::sig_resetTime,this,&MotionController::sig_resetTime);

    connect(m_mManger,&MotionManager::sig_dataChanged,this,&MotionController::sig_dataChanged);

    connect(m_mManger,&MotionManager::sig_DIStatusdataChanged,this,&MotionController::sig_DIStatusdataChanged);

    connect(m_mManger,&MotionManager::sig_motionMsg,this,&MotionController::sig_motionMsg);


    connect(this,&MotionController::sig_beamContinuousScanning,m_mManger,&MotionManager::sig_beamContinuousScanning);
}
