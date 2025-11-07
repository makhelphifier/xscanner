#include "motionctrlwidget.h"
#include "motionctrlviewmodel.h"
#include "parasettingsdialog.h"
#include "othercontrolsdialog.h"
#include <QMessageBox>
#include <QTimer>
#include "ui_motionctrlwidget.h"


MotionCtrlWidget::MotionCtrlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MotionCtrlWidget)
{
    this->setAttribute(Qt::WA_StyledBackground);
    ui->setupUi(this);

    msgBox.setStyleSheet("QMessageBox{background: #232323; padding-left: 20px; padding-right: 20px;}"
                         "QMessageBox QPushButton{border-radius: 4px; padding-left: 24px; padding-right: 24px; padding-top: 4px; padding-bottom: 4px; background: #2399ff; font-family: 'Microsoft YaHei'; font-size: 14px; color: #cdcdcd;}"
                         "QMessageBox QPushButton::hover{border-color: white; color: white;}"
                         "QMessageBox QLabel{font-family: 'Microsoft YaHei'; font-size: 16px; color: white;}");

    msgBox.setWindowFlags(msgBox.windowFlags() | Qt::FramelessWindowHint);
    msgBox.setStandardButtons(QMessageBox::NoButton);

    mcViewModel = new MotionCtrlViewModel(this);
    paraSettingsDialog = new ParaSettingsDialog();
    otherControlsDialog = new OtherControlsDialog();
    axisParamsDialog = new AxisParametersDialog();
    cardIO_ = new cardIO();
    connections();
    appcinfig = new AppConfig;
    appcinfig->readConfig();

    float axisPos = appcinfig->getAxisPos(AXIS::Detector_R);
    ui->edit_pos1->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::XRAY_Z);
    ui->edit_pos2->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::objectiveTable_X1);
    ui->edit_pos3->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::Detector_Z1);
    ui->edit_pos4->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::objectiveTable_Y1);
    ui->edit_pos5->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::Detector_Z2);
    ui->edit_pos6->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::objectiveTable_X2);
    ui->edit_pos7->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::Detector_W);
    ui->edit_pos14->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::objectiveTable_Y2);
    ui->edit_pos13->setText(QString::number(axisPos,'f',4));
    axisPos = appcinfig->getAxisPos(AXIS::XRAY_Z);
    ui->edit_pos8->setText(QString::number(axisPos,'f',4));

    ui->btn_test->setVisible(false);

    QString speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::Detector_R+1));
    ui->speed1->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::XRAY_Z+1));
    ui->speed2->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::objectiveTable_X1+1));
    ui->speed3->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::Detector_Z1+1));
    ui->speed4->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::objectiveTable_Y1+1));
    ui->speed5->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::Detector_Z2+1));
    ui->speed6->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::objectiveTable_X2+1));
    ui->speed7->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::Detector_W+1));
    ui->speed14->setText(speed);
    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::objectiveTable_Y2+1));
    ui->speed13->setText(speed);

    speed = mcViewModel->getValueFromIni("MotionCtrl/ddSpeed" + QString::number(AXIS::XRAY_Z+1));
    ui->speed8->setText(speed);


    ui->btn_otherCtrl->setVisible(false);

    ui->tabWidget->setTabVisible(3,false);
    ui->label_9->setVisible(false);
    ui->lineEdit->setVisible(false);
    ui->groupBox_8->setVisible(false);
    ui->groupBox_14->setVisible(false);
    ui->groupBox_3->setVisible(false);
    ui->groupBox_12->setVisible(false);


    QSizePolicy retain = ui->groupBox_8->sizePolicy();
    retain.setRetainSizeWhenHidden(true);
    ui->groupBox_8->setSizePolicy(retain);

    retain = ui->groupBox_14->sizePolicy();
    retain.setRetainSizeWhenHidden(true);
    ui->groupBox_14->setSizePolicy(retain);

    retain = ui->groupBox_3->sizePolicy();
    retain.setRetainSizeWhenHidden(true);
    ui->groupBox_3->setSizePolicy(retain);

    retain = ui->groupBox_12->sizePolicy();
    retain.setRetainSizeWhenHidden(true);
    ui->groupBox_12->setSizePolicy(retain);



    // --- 新增：根据初始“目标位置”计算并设置初始“目标坐标” ---
    float initial_pos, initial_coord;
    AppConfig config;
    config.readConfig(); // 确保加载了最新的系数和补偿值

    // 探测器R轴
    initial_pos = ui->edit_pos1->text().toFloat();
    if (AppConfig::detectorRCoefficient != 0) {
        initial_coord = (initial_pos - AppConfig::detectorROffset) / AppConfig::detectorRCoefficient;
        ui->edit_target_coord_1->setText(QString::number(initial_coord, 'f', 4));
    }

    // 射线源Z轴
    initial_pos = ui->edit_pos2->text().toFloat();
    if (AppConfig::xrayZCoefficient != 0) {
        initial_coord = (initial_pos - AppConfig::xrayZOffset) / AppConfig::xrayZCoefficient;
        ui->edit_target_coord_2->setText(QString::number(initial_coord, 'f', 4));
    }

    // 载物台X轴
    initial_pos = ui->edit_pos3->text().toFloat();
    if (AppConfig::tableXCoefficient != 0) {
        initial_coord = (initial_pos - AppConfig::tableXOffset) / AppConfig::tableXCoefficient;
        ui->edit_target_coord_3->setText(QString::number(initial_coord, 'f', 4));
    }

    // 探测器Z1轴
    initial_pos = ui->edit_pos4->text().toFloat();
    if (AppConfig::detectorZCoefficient != 0) {
        initial_coord = (initial_pos - AppConfig::detectorZOffset) / AppConfig::detectorZCoefficient;
        ui->edit_target_coord_4->setText(QString::number(initial_coord, 'f', 4));
    }

    // 载物台Y轴
    initial_pos = ui->edit_pos5->text().toFloat();
    if (AppConfig::tableYCoefficient != 0) {
        initial_coord = (initial_pos - AppConfig::tableYOffset) / AppConfig::tableYCoefficient;
        ui->edit_target_coord_5->setText(QString::number(initial_coord, 'f', 4));
    }

    // 探测器theta轴
    initial_pos = ui->edit_pos14->text().toFloat();
    if (AppConfig::detectorThetaCoefficient != 0) {
        initial_coord = (initial_pos - AppConfig::detectorThetaOffset) / AppConfig::detectorThetaCoefficient;
        ui->edit_target_coord_14->setText(QString::number(initial_coord, 'f', 4));
    }




    // 隐藏旧的“当前位置”和“目标位置”控件 ---

    // // 射线源Z轴
    // ui->lb_axisNo4_10->setVisible(false);
    // ui->label_2->setVisible(false);
    // ui->lb_pos4_15->setVisible(false);
    // ui->edit_pos2->setVisible(false);

    // // 探测器Z1轴
    // ui->lb_axisNo4_7->setVisible(false);
    // ui->label_4->setVisible(false);
    // ui->lb_pos4_9->setVisible(false);
    // ui->edit_pos4->setVisible(false);

    // // 探测器R轴
    // ui->lb_axisNo4_9->setVisible(false);
    // ui->label->setVisible(false);
    // ui->lb_pos4_13->setVisible(false);
    // ui->edit_pos1->setVisible(false);

    // // 探测器theta轴
    // ui->lb_axisNo4_16->setVisible(false);
    // ui->label_14->setVisible(false);
    // ui->lb_pos4_27->setVisible(false);
    // ui->edit_pos14->setVisible(false);

    // // 载物台X轴
    // ui->lb_axisNo4_4->setVisible(false);
    // ui->label_3->setVisible(false);
    // ui->lb_pos4_3->setVisible(false);
    // ui->edit_pos3->setVisible(false);

    // // 载物台Y轴
    // ui->lb_axisNo4_5->setVisible(false);
    // ui->label_5->setVisible(false);
    // ui->lb_pos4_5->setVisible(false);
    // ui->edit_pos5->setVisible(false);
}

MotionCtrlWidget::~MotionCtrlWidget()
{
    qDebug()<<"~MotionCtrlWidget";
    delete ui;
   for(int i = 8 ;i<50;i++)
    {
        MotionCtrlManager::getInstance()->mcCommand->writeOutbit(i,1);//设置灯带绿
    }
    paraSettingsDialog->deleteLater();
    otherControlsDialog->deleteLater();
    axisParamsDialog->deleteLater();
    cardIO_->deleteLater();
}

void MotionCtrlWidget::connections()
{

    ui->label->setFrameShape(QFrame::Box);
    ui->label_2->setFrameShape(QFrame::Box);
    ui->label_3->setFrameShape(QFrame::Box);
    ui->label_4->setFrameShape(QFrame::Box);
    ui->label_5->setFrameShape(QFrame::Box);
    ui->label_6->setFrameShape(QFrame::Box);
    ui->label_7->setFrameShape(QFrame::Box);
    ui->label_8->setFrameShape(QFrame::Box);
    ui->label_9->setFrameShape(QFrame::Box);
    ui->label_10->setFrameShape(QFrame::Box);
    ui->label_11->setFrameShape(QFrame::Box);
    ui->label_12->setFrameShape(QFrame::Box);
    ui->label_13->setFrameShape(QFrame::Box);
    ui->label_14->setFrameShape(QFrame::Box);

    //=====================👇 ============================
    //轴0 速度
    connect(ui->speed1,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::Detector_R,ui->speed1->text(),ui->speed1->text());
    });
    //轴0 jog 左键按下 运动
    connect(ui->btn_left1,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::Detector_R);
    });

    //轴0 jog 左键释放 停止
    connect(ui->btn_left1,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_R);
    });

    //轴0 jog 右键按下 运动
    connect(ui->btn_right1,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::Detector_R);
    });

    //轴0 jog 右键释放 停止
    connect(ui->btn_right1,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_R);
    });

    //轴0 停止
    connect(ui->btn_stop1,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_R);
    });


    //轴0 绝对位置  回车事件
    connect(ui->edit_pos1,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::Detector_R,ui->edit_pos1->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::Detector_R,ui->edit_pos1->text());
    });

    //=====================👆 ============================


    //=====================👇 ============================
    //轴1 速度
    connect(ui->speed2,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::XRAY_Z,ui->speed2->text(),ui->speed2->text());
    });
    //轴1 jog 左键按下 运动
    connect(ui->btn_left2,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::XRAY_Z);
    });

    //轴1 jog 左键释放 停止
    connect(ui->btn_left2,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::XRAY_Z);
    });

    //轴1 jog 右键按下 运动
    connect(ui->btn_right2,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::XRAY_Z);
    });

    //轴1 jog 右键释放 停止
    connect(ui->btn_right2,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::XRAY_Z);
    });

    //轴1 停止
    connect(ui->btn_stop2,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::XRAY_Z);
    });


    //轴1 绝对位置  回车事件
    connect(ui->edit_pos2,&QLineEdit::returnPressed,this,[this](){

        appcinfig->setAxisPos(AXIS::XRAY_Z,ui->edit_pos2->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::XRAY_Z,ui->edit_pos2->text());
    });

    //=====================👆 ============================

    //=====================👇 ============================
    //轴2 速度
    connect(ui->speed3,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::objectiveTable_X1,ui->speed3->text(),ui->speed3->text());
    });

    //轴2 jog 左键按下 运动
    connect(ui->btn_left3,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::objectiveTable_X1);
    });

    //轴2 jog 左键释放 停止
    connect(ui->btn_left3,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_X1);
    });

    //轴2 jog 右键按下 运动
    connect(ui->btn_right3,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::objectiveTable_X1);
    });

    //轴2 jog 右键释放 停止
    connect(ui->btn_right3,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_X1);
    });

    //轴2 停止
    connect(ui->btn_stop3,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_X1);
    });


    //轴2 绝对位置  开始运动 回车事件
    connect(ui->edit_pos3,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::objectiveTable_X1,ui->edit_pos3->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::objectiveTable_X1,ui->edit_pos3->text());
    });
    //=====================👆 ============================

    //=====================👇 ============================
    //轴3 速度
    connect(ui->speed4,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::Detector_Z1,ui->speed4->text(),ui->speed4->text());
    });

    //轴3 jog 左键按下 运动
    connect(ui->btn_left4,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::Detector_Z1);
    });

    //轴3 jog 左键释放 停止
    connect(ui->btn_left4,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_Z1);
    });

    //轴3 jog 右键按下 运动
    connect(ui->btn_right4,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::Detector_Z1);
    });

    //轴3 jog 右键释放 停止
    connect(ui->btn_right4,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_Z1);
    });

    //轴3 停止
    connect(ui->btn_stop4,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_Z1);
    });


    //轴3 绝对位置  开始运动 回车事件
    connect(ui->edit_pos4,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::Detector_Z1,ui->edit_pos4->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::Detector_Z1,ui->edit_pos4->text());
    });
    //=====================👆 ============================

    //=====================👇 ============================
    //轴4 速度
    connect(ui->speed5,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::objectiveTable_Y1,ui->speed5->text(),ui->speed5->text());
    });

    //轴4 jog 左键按下 运动
    connect(ui->btn_left5,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::objectiveTable_Y1);
    });

    //轴4 jog 左键释放 停止
    connect(ui->btn_left5,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_Y1);
    });

    //轴4 jog 右键按下 运动
    connect(ui->btn_right5,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::objectiveTable_Y1);
    });

    //轴4 jog 右键释放 停止
    connect(ui->btn_right5,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_Y1);
    });

    //轴4 停止
    connect(ui->btn_stop5,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_Y1);
    });


    //轴4 绝对位置  开始运动 回车事件
    connect(ui->edit_pos5,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::objectiveTable_Y1,ui->edit_pos5->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::objectiveTable_Y1,ui->edit_pos5->text());
    });
    //=====================👆 ============================
    //=====================👇 ============================
    //轴5 速度
    connect(ui->speed6,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::Detector_Z2,ui->speed6->text(),ui->speed6->text());
    });

    //轴5 jog 左键按下 运动
    connect(ui->btn_left6,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::Detector_Z2);
    });

    //轴5 jog 左键释放 停止
    connect(ui->btn_left6,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_Z2);
    });

    //轴5 jog 右键按下 运动
    connect(ui->btn_right6,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::Detector_Z2);
    });

    //轴5 jog 右键释放 停止
    connect(ui->btn_right6,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_Z2);
    });

    //轴5 停止
    connect(ui->btn_stop6,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_Z2);
    });

    //轴5 绝对位置  开始运动 回车事件
    connect(ui->edit_pos6,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::Detector_Z2,ui->edit_pos6->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::Detector_Z2,ui->edit_pos6->text());
    });
    //=====================👆 ============================

    //=====================👇 ============================
    //轴6 速度
    connect(ui->speed7,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::objectiveTable_X2,ui->speed7->text(),ui->speed7->text());
    });

    //轴6 jog 左键按下 运动
    connect(ui->btn_left7,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::objectiveTable_X2);
    });

    //轴6 jog 左键释放 停止
    connect(ui->btn_left7,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_X2);
    });

    //轴6 jog 右键按下 运动
    connect(ui->btn_right7,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::objectiveTable_X2);
    });

    //轴6 jog 右键释放 停止
    connect(ui->btn_right7,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_X2);
    });

    //轴6 绝对位置  开始运动
    connect(ui->btn_stop7,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_X2);
    });

    //轴6 绝对位置  开始运动 回车事件
    connect(ui->edit_pos7,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::objectiveTable_X2,ui->edit_pos7->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::objectiveTable_X2,ui->edit_pos7->text());
    });
    //=====================👆 ============================

    //=====================👇 ============================
    //轴7 速度
    connect(ui->speed14,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::Detector_W,ui->speed14->text(),ui->speed14->text());
    });

    //轴7 jog 左键按下 运动
    connect(ui->btn_left14,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::Detector_W);
    });

    //轴7 jog 左键释放 停止
    connect(ui->btn_left14,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_W);
    });

    //轴7 jog 右键按下 运动
    connect(ui->btn_right14,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::Detector_W);
    });

    //轴7 jog 右键释放 停止
    connect(ui->btn_right14,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_W);
    });

    //轴7 绝对位置  开始运动
    connect(ui->btn_stop14,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::Detector_W);
    });

    //轴7 绝对位置  开始运动 回车事件
    connect(ui->edit_pos14,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::Detector_W,ui->edit_pos14->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::Detector_W,ui->edit_pos14->text());
    });
    //=====================👆 ============================
    //=====================👇 ============================
    //轴8 速度
    connect(ui->speed13,&QLineEdit::returnPressed,this,[this](){
        mcViewModel->saveSpeed(AXIS::objectiveTable_Y2,ui->speed13->text(),ui->speed13->text());
    });

    //轴8 jog 左键按下 运动
    connect(ui->btn_left13,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisLeftPressed(AXIS::objectiveTable_Y2);
    });

    //轴8 jog 左键释放 停止
    connect(ui->btn_left13,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_Y2);
    });

    //轴8 jog 右键按下 运动
    connect(ui->btn_right13,&QPushButton::pressed,this,[this](){
        mcViewModel->onBtn_axisRightPressed(AXIS::objectiveTable_Y2);
    });

    //轴8 jog 右键释放 停止
    connect(ui->btn_right13,&QPushButton::released,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_Y2);
    });

    //轴8 绝对位置  开始运动
    connect(ui->btn_stop13,&QPushButton::clicked,this,[this](){
        mcViewModel->onBtn_axisStopClicked(AXIS::objectiveTable_Y2);
    });

    //轴8 绝对位置  开始运动 回车事件
    connect(ui->edit_pos13,&QLineEdit::returnPressed,this,[this](){
        appcinfig->setAxisPos(AXIS::objectiveTable_Y2,ui->edit_pos13->text().toFloat());

        mcViewModel->onBtn_axisStartClicked(AXIS::objectiveTable_Y2,ui->edit_pos13->text());
    });
    //=====================👆 ============================


    //其他操作 开门、硬复位、软复位
    connect(ui->home1,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::Detector_R);
    });

    connect(ui->btn_home2,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::XRAY_Z);
    });

    connect(ui->home3,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::objectiveTable_X1);
    });

    connect(ui->btn_home4,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::Detector_Z1);
    });

    connect(ui->home5,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::objectiveTable_Y1);
    });

    connect(ui->home6,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::Detector_Z2);
    });
    connect(ui->home7,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::objectiveTable_X2);
    });
    connect(ui->btn_home8,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::XRAY_Z);
        mcViewModel->onBtn_axisResetPosClicked(AXIS::Detector_R);
    });
    connect(ui->home14,&QPushButton::clicked,this,[this]( ){
        mcViewModel->onBtn_axisResetPosClicked(AXIS::Detector_W);

    });


    connect(ui->btn_otherCtrl,&QPushButton::clicked,otherControlsDialog,&OtherControlsDialog::show);
    connect(ui->btn_IO,&QPushButton::clicked,cardIO_,&cardIO::show);

    axisFormula_ = new AxisFormula();
    connect(ui->btn_test_2,&QPushButton::clicked,axisFormula_,&AxisFormula::show/*&AxisPos::show*/);
    connect(axisFormula_,&AxisFormula::sig_interpositionAxis,this,[this](){
        axisFormula_->slotSetinterpositionAxis(ui->label_3->text(),ui->label_5->text());
    });

    connect(axisFormula_,&AxisFormula::sig_getaxis,this,[this](){
        axisFormula_->slotSetAxis(ui->label_3->text(),ui->label_5->text());
    });

    connect(paraSettingsDialog,&ParaSettingsDialog::sig_saveSpeed,mcViewModel,&MotionCtrlViewModel::saveSpeed);

    connect(otherControlsDialog,&OtherControlsDialog::sig_resetPos,this,[this](int value){
        mcViewModel->onBtn_axisResetPosClicked(value);
    });

    connect(otherControlsDialog,&OtherControlsDialog::sig_enable,this,[this](int value,bool status){
        mcViewModel->onBtn_axisEnableClicked(value,status);
    });

    connect(otherControlsDialog,&OtherControlsDialog::sig_stop,this,[this](int value){
        mcViewModel->onBtn_axisStopClicked(value);
    });

    connect(mcViewModel,&MotionCtrlViewModel::sig_resetTime,this,[this](int value){
        if(value == 0) {
            msgBox.accept();
            return;
        }
        msgBox.setText(u8"设备正在进行复位，请" + QString::number(value) + "s后再进行操作!");
    });

    connect(mcViewModel,&MotionCtrlViewModel::sig_enableChanged,otherControlsDialog,&OtherControlsDialog::on_enableChanged);

    connect(mcViewModel,&MotionCtrlViewModel::sig_posLimit,this,&MotionCtrlWidget::on_PosLimit);


    mcManger = MotionCtrlManager::getInstance();
    connect(mcManger,&MotionCtrlManager::sig_dataChanged,this,&MotionCtrlWidget::slot_DataChanged);


    //=====================👇 新增：目标坐标关联到目标位置 👇============================

    // 射线源Z轴: 目标坐标 -> 目标位置
    connect(ui->edit_target_coord_2, &QLineEdit::returnPressed, this, [this](){
        float coord = ui->edit_target_coord_2->text().toFloat();
        float pos = coord * AppConfig::xrayZCoefficient + AppConfig::xrayZOffset;
        ui->edit_pos2->setText(QString::number(pos, 'f', 4));
        emit ui->edit_pos2->returnPressed();
    });

    // 探测器Z1轴: 目标坐标 -> 目标位置
    connect(ui->edit_target_coord_4, &QLineEdit::returnPressed, this, [this](){
        float coord = ui->edit_target_coord_4->text().toFloat();
        float pos = coord * AppConfig::detectorZCoefficient + AppConfig::detectorZOffset;
        ui->edit_pos4->setText(QString::number(pos, 'f', 4));
        emit ui->edit_pos4->returnPressed();
    });

    // 探测器R轴: 目标坐标 -> 目标位置
    connect(ui->edit_target_coord_1, &QLineEdit::returnPressed, this, [this](){
        float coord = ui->edit_target_coord_1->text().toFloat();
        float pos = coord * AppConfig::detectorRCoefficient + AppConfig::detectorROffset;
        ui->edit_pos1->setText(QString::number(pos, 'f', 4));
        emit ui->edit_pos1->returnPressed();
    });

    // 探测器θ轴: 目标坐标 -> 目标位置
    connect(ui->edit_target_coord_14, &QLineEdit::returnPressed, this, [this](){
        float coord = ui->edit_target_coord_14->text().toFloat();
        float pos = coord * AppConfig::detectorThetaCoefficient + AppConfig::detectorThetaOffset;
        ui->edit_pos14->setText(QString::number(pos, 'f', 4));
        emit ui->edit_pos14->returnPressed();
    });

    // 载物台X轴: 目标坐标 -> 目标位置
    connect(ui->edit_target_coord_3, &QLineEdit::returnPressed, this, [this](){
        float coord = ui->edit_target_coord_3->text().toFloat();
        float pos = coord * AppConfig::tableXCoefficient + AppConfig::tableXOffset;
        ui->edit_pos3->setText(QString::number(pos, 'f', 4));
        emit ui->edit_pos3->returnPressed();
    });

    // 载物台Y轴: 目标坐标 -> 目标位置
    connect(ui->edit_target_coord_5, &QLineEdit::returnPressed, this, [this](){
        float coord = ui->edit_target_coord_5->text().toFloat();
        float pos = coord * AppConfig::tableYCoefficient + AppConfig::tableYOffset;
        ui->edit_pos5->setText(QString::number(pos, 'f', 4));
        emit ui->edit_pos5->returnPressed();
    });

    //=====================👆 新增：目标坐标关联到目标位置 👆============================

    // connect(ui->btn_params, &QPushButton::clicked, axisParamsDialog, &AxisParametersDialog::show);

    connect(ui->btn_params, &QPushButton::clicked, axisParamsDialog, [this](){
        // 如果窗口是第一次创建，则新建一个
        if (axisParamsDialog == nullptr) {
            axisParamsDialog = new AxisParametersDialog(this); // 将主窗口设为父对象
        }

        // 如果窗口被最小化了或者隐藏了，则正常显示
        if (axisParamsDialog->isMinimized()) {
            axisParamsDialog->showNormal();
        } else {
            axisParamsDialog->show();
        }

        // 将窗口提升到顶层
        axisParamsDialog->raise();
        // 激活窗口，使其获得焦点
        axisParamsDialog->activateWindow();
    });
}

void MotionCtrlWidget::showMessageBox()
{
//    QTimer timer;
//    timer.setInterval(1000);
//    static int cnt = 15;
//    connect(&timer,&QTimer::timeout,this,[this,&timer](){
//        if(cnt == 0) {
//            msgBox.accept();
//            timer.stop();
//            return;
//        }
//        msgBox.setText(u8"设备正在进行复位，请" + QString::number(cnt) + "s后再进行操作!");
//        cnt--;
//    });
//    timer.start();
    // msgBox.exec();
}


void MotionCtrlWidget::on_btn_test_clicked()
{

    mcViewModel->beamContinuousScanning();
}

//清除轴错误
void MotionCtrlWidget::on_btn_IO_2_clicked()
{
    mcViewModel->onBtn_ClearAxisErrcode();
}

void MotionCtrlWidget::on_PosLimit()
{
    QMessageBox::information(nullptr, u8"轴1 提示", u8"到达限位，请向相反方向移动");
}

void MotionCtrlWidget::setStartAxis(bool val)
{
    if(val)
    {
        mcViewModel->onBtn_axisStartXYClicked();
        MotionCtrlManager::getInstance()->mcCommand->vecStartFlag = true;
    }else
    {
        MotionCtrlManager::getInstance()->mcCommand->vecStartFlag = false;
        mcViewModel->onBtn_axisStopClicked(255);
    }
}

void MotionCtrlWidget::slot_DataChanged(const QVariant &var)
{
    QVector<MotionCtrlData::MotionCtrlInfo> mcVec = var.value<QVector<MotionCtrlData::MotionCtrlInfo>>();
    int size = mcVec.size();

    float curdist =0;

    // qDebug()<<"slot_DataChanged size:"<<size;
    for (int i = 0; i < size; ++i) {
        float currentPos = mcVec[i].axEncPos;
        float currentCoord = 0.0;
        switch (i) {
        case AXIS::Detector_R:
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left1->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right1->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left1->setEnabled(false);
                ui->btn_right1->setEnabled(false);
            }else
            {
                ui->btn_left1->setEnabled(true);
                ui->btn_right1->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->home1->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->home1->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label->setText(QString::number(mcVec[i].axEncPos,'f',4));
            // 更新坐标位置
            if (AppConfig::detectorRCoefficient != 0) {
                currentCoord = (currentPos - AppConfig::detectorROffset) / AppConfig::detectorRCoefficient;
            }
            ui->label_coord_pos_1->setText(QString::number(currentCoord,'f',4));
            break;
        case AXIS::XRAY_Z:
            //正限位、负限位、 伺服报警、
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left2->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right2->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left2->setEnabled(false);
                ui->btn_right2->setEnabled(false);
            }else
            {
                ui->btn_left2->setEnabled(true);
                ui->btn_right2->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->btn_home2->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->btn_home2->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label_2->setText(QString::number(mcVec[i].axEncPos,'f',4));
            ui->label_8->setText(QString::number(mcVec[i].axEncPos,'f',4));
            // 更新坐标位置
            if (AppConfig::xrayZCoefficient != 0) {
                currentCoord = (currentPos - AppConfig::xrayZOffset) / AppConfig::xrayZCoefficient;
            }
            ui->label_coord_pos_2->setText(QString::number(currentCoord,'f',4));
            break;
        case AXIS::objectiveTable_X1:
            //正限位、负限位、 伺服报警、
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left3->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right3->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left3->setEnabled(false);
                ui->btn_right3->setEnabled(false);
            }else
            {
                ui->btn_left3->setEnabled(true);
                ui->btn_right3->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->home3->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->home3->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label_3->setText(QString::number(mcVec[i].axEncPos,'f',4));
            // 更新坐标位置
            if (AppConfig::tableXCoefficient != 0) {
                currentCoord = (currentPos - AppConfig::tableXOffset) / AppConfig::tableXCoefficient;
            }
            ui->label_coord_pos_3->setText(QString::number(currentCoord,'f',4));
            break;

        case AXIS::Detector_Z1:
            //正限位、负限位、 伺服报警、
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left4->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right4->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left4->setEnabled(false);
                ui->btn_right4->setEnabled(false);
            }else
            {
                ui->btn_left4->setEnabled(true);
                ui->btn_right4->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->btn_home4->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->btn_home4->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label_4->setText(QString::number(mcVec[i].axEncPos,'f',4));
            // 更新坐标位置
            if (AppConfig::detectorZCoefficient != 0) {
                currentCoord = (currentPos - AppConfig::detectorZOffset) / AppConfig::detectorZCoefficient;
            }
            ui->label_coord_pos_4->setText(QString::number(currentCoord,'f',4));
            break;
        case AXIS::objectiveTable_Y1:
            //正限位、负限位、 伺服报警、
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left5->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right5->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left5->setEnabled(false);
                ui->btn_right5->setEnabled(false);
            }else
            {
                ui->btn_left5->setEnabled(true);
                ui->btn_right5->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->home5->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->home5->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label_5->setText(QString::number(mcVec[i].axEncPos,'f',4));
            // qDebug()<<currentCoord<<"=========currentCoord"<<mcVec[i].axEncPos<<"=====mcVec[i].axEncPos";
            // 更新坐标位置
            if (AppConfig::tableYCoefficient != 0) {
                currentCoord = (currentPos - AppConfig::tableYOffset) / AppConfig::tableYCoefficient;
            }
            // qDebug()<<currentCoord<<"=========currentCoord"<<mcVec[i].axEncPos<<"=====mcVec[i].axEncPos";

            ui->label_coord_pos_5->setText(QString::number(currentCoord,'f',4));
            break;
        case AXIS::Detector_Z2:
            // curdist = mcVec[5].axEncPos - mcVec[5].curdist;
            // ui->label_6->setText(QString::number(curdist,'f',4));
            break;
        case AXIS::objectiveTable_X2:
            //正限位、负限位、 伺服报警、
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left7->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right7->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left7->setEnabled(false);
                ui->btn_right7->setEnabled(false);
            }else
            {
                ui->btn_left7->setEnabled(true);
                ui->btn_right7->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->home7->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->home7->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label_7->setText(QString::number(mcVec[i].axEncPos,'f',4));
            break;
        case AXIS::Detector_W:// 这个是探测器 theta 轴
            //正限位、负限位、 伺服报警、
            if(mcVec[i].hardNegLmtWarn) {
                //到达限位处理
                ui->btn_left14->setEnabled(false);
            } else if(mcVec[i].hardPosLmtWarn)
            {
                ui->btn_right14->setEnabled(false);
            } else if(mcVec[i].almWarn)
            {
                ui->btn_left14->setEnabled(false);
                ui->btn_right14->setEnabled(false);
            }else
            {
                ui->btn_left14->setEnabled(true);
                ui->btn_right14->setEnabled(true);
            }
            if(mcVec[i].ZeroingIsComplete)
            {
                ui->home14->setStyleSheet("background: #2793ff;");// border-radius: 7px;
            }else
            {
                ui->home14->setStyleSheet("background: #ff3a3a; ");
            }
            ui->label_14->setText(QString::number(mcVec[i].axEncPos,'f',4));
            // 更新坐标位置
            if (AppConfig::detectorThetaCoefficient != 0) {
                currentCoord = (currentPos - AppConfig::detectorThetaOffset) / AppConfig::detectorThetaCoefficient;
            }
            ui->label_coord_pos_14->setText(QString::number(currentCoord,'f',4));
            break;
        case AXIS::objectiveTable_Y2:
            ui->label_13->setText(QString::number(mcVec[i].axEncPos,'f',4));
            break;
        default:
            break;
        }

    }
}


void MotionCtrlWidget::on_lineEdit_returnPressed()
{
    double frame = ui->lineEdit->text().toDouble();
    MotionCtrlManager::getInstance()->mcCommand->pwmOutput(frame);
}

