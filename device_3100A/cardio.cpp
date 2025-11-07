#include "cardio.h"
#include "ui_cardio.h"
#include "motionctrlmanager.h"
cardIO::cardIO(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::cardIO)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    // qRegisterMetaType<QVector<DIStatus>>("QVector<DIStatus>");
    connect(MotionCtrlManager::getInstance()->mcCommand,&MotionCtrlCommand::sig_DIStatusdataChanged,this,&cardIO::slot_DataChanged);

    connect(ui->btn_LDoor,&QPushButton::clicked,this,[this](){
        ushort val = 0;

        MotionCtrlManager::getInstance()->mcCommand->readOutbit(cardDO::LEFTDOORLOCKBTN,&val);
        MotionCtrlManager::getInstance()->mcCommand->writeOutbit(cardDO::LEFTDOORLOCKBTN,!val);
    });
    connect(ui->btn_RDoor,&QPushButton::clicked,this,[this](){
        ushort val = 0;

        MotionCtrlManager::getInstance()->mcCommand->readOutbit(cardDO::RIGHTDOORLOCKBTN,&val);
        MotionCtrlManager::getInstance()->mcCommand->writeOutbit(cardDO::RIGHTDOORLOCKBTN,!val);
    });
    connect(ui->btn_Door,&QPushButton::clicked,this,[this](){
        ushort val = 0;

        MotionCtrlManager::getInstance()->mcCommand->readOutbit(cardDO::DOORLOCKBTN,&val);
        MotionCtrlManager::getInstance()->mcCommand->writeOutbit(cardDO::DOORLOCKBTN,!val);
    });
    //软复位
    connect(ui->btn_softReset,&QPushButton::clicked,this,[this](){
          MotionCtrlManager::getInstance()->softReset();
    });
    //硬复位
    connect(ui->btn_boardReset,&QPushButton::clicked,this,[this](){
        MotionCtrlManager::getInstance()->boardReset();
    });
    //一键回零  所有轴
    connect(ui->btn_allHoming,&QPushButton::clicked,this,[this](){
        MotionCtrlManager::getInstance()->mcCommand->axisAllHomingMove();
    });
}

cardIO::~cardIO()
{
    delete ui;
}

void cardIO::on_btn_close_clicked()
{
    this->close();
}

//获取io的状态
void cardIO::slot_DataChanged(const QVariant &var)
{
    QVector<DIStatus> diVec = var.value<QVector<DIStatus>>();
    int svSize = diVec.size();

    for (int i = 8; i < svSize; ++i) {
        // qDebug()<<__FUNCTION__<<"svVec i :"<<i<<" value : "<<diVec[i].IO_Port <<"   " <<!diVec[i].IO_Status;
        switch (diVec[i].IO_Port) {
        case cardDI::XRAYCONDITIONSMET:
        {
            if(!diVec[i].IO_Status)
            {
                ui->xray_label->setStyleSheet("background: #55ff00; border: none; border-radius: 4px;");

            }
            else
            {
                ui->xray_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
            }

        }
        break;
        case cardDI::STOP:
        {
            if(!diVec[i].IO_Status)
            {
                ui->stop_label->setStyleSheet("background: #55ff00; border: none; border-radius: 4px;");

            }
            else
            {
                // emit sig_ioOpenDoor(true);//此时门是开着的，开门应该可以按下
                ui->stop_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
                // MotionCtrlManager::getInstance()->emgStop();
            }

        }
        break;
        case cardDI::RESET:
        {
            if(!diVec[i].IO_Status)
            {
                ui->reset_label->setStyleSheet("background: #55ff00; border: none; border-radius: 4px;");
                // MotionCtrlManager::getInstance()->emgStop();
            }
            else
                ui->reset_label->setStyleSheet("background: #8d8d8d; border: none; border-radius: 4px;");
        }
        break;
        case cardDI::EMGSTOP1:
        {
            if(!diVec[i].IO_Status)
            {
                ui->emgstop1_label->setStyleSheet("background: #55ff00; border: none; border-radius: 4px;");

            }
            else
            {
                ui->emgstop1_label->setStyleSheet("background: #8d8d8d; border: none; border-radius: 4px;");
                // MotionCtrlManager::getInstance()->emgStop();
            }

        }
        break;
        case cardDI::EMGSTOP2:
        {
            if(!diVec[i].IO_Status)
            {
                ui->emgstop2_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
            {
                ui->emgstop2_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
                // MotionCtrlManager::getInstance()->emgStop();
            }

        }
        break;
        case cardDI::DOORBOLT:
        {
            if(!diVec[i].IO_Status)
            {
                ui->blot_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
                ui->blot_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
        }
        break;
        case cardDI::RIGHTDOORBOLT:
        {
            if(!diVec[i].IO_Status)
            {
                ui->blot1_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
                ui->blot1_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
        }
        break;

        case cardDI::LEFTDOORBOLT:
        {
            if(!diVec[i].IO_Status)
            {
                ui->blot2_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
                ui->blot2_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
        }
        break;
        case cardDI::DOORLOCK:
        {
            if(!diVec[i].IO_Status)
            {
                doorLock = true;
                ui->lock_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
                ui->lock_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
        }
        break;
        case cardDI::RIGHTDOORLOCK:
        {
            if(!diVec[i].IO_Status)
            {
                backDoorLock = true;
                ui->lock1_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
                ui->lock1_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
        }
        break;
        case cardDI::LEFTDOORLOCK:
        {
            if(!diVec[i].IO_Status)
            {
                leftDoorLock = true;
                ui->lock2_label->setStyleSheet("background:  #55ff00; border: none; border-radius: 4px;");
            }
            else
                ui->lock2_label->setStyleSheet("background:  #8d8d8d; border: none; border-radius: 4px;");
        }
        break;
        default:
            break;
        }
    }

    if(doorLock && leftDoorLock && backDoorLock)
    {
        QTimer::singleShot(5000,[this](){

            ushort val = 0;
            MotionCtrlManager::getInstance()->mcCommand->writeOutbit(cardDO::XRAYSAFETYCIRCUIT,val);

            doorLock = false;
            leftDoorLock = false;
            backDoorLock = false;
        });
    }
    else
    {
            ushort val = 1;
            MotionCtrlManager::getInstance()->mcCommand->writeOutbit(cardDO::XRAYSAFETYCIRCUIT,val);
    }
}



