#include "motionwidget.h"
#include "xraysource.h"
#include "motioncontroller.h"
#include "motionconfig.h"
#include "motiondef.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QTimer>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>


GongYTabWidget::GongYTabWidget(QWidget *parent)
    : QTabWidget(parent), m_opacity(1.0),m_font("Microsoft YaHei"),m_online(false)
{
    setupTabs();

    // 连接当前切换信号
    connect(this, SIGNAL(currentchanged(int)), this, SLOT(oncurrentchanged(int)));

    m_pXraySource = XraySource::getInstance();
    connect(m_pXraySource, &XraySource::sig_didGetXrayStatus, this, &GongYTabWidget::slotGetXrayStatus);
    connect(m_pXraySource, &XraySource::sig_didConnect, this, [this](){
        m_pConnect->setChecked(true);
        m_online = true;
    });
    connect(m_pXraySource, &XraySource::sig_didDisconnect, this, [this](int state){
        m_pConnect->setChecked(false);
        m_online=false;
    });
    m_pXraySource->startToConnect(0);

    m_pMotionController = new MotionController(this);
    connect(m_pMotionController,&MotionController::sig_motionMsg,this,[this](QString msg){
        // ui->textEdit->append(msg);
    });
    connect(m_pMotionController,&MotionController::sig_motionMsg,this,[this](QString msg){
        // ui->textEdit->append(msg);
    });
    connect(m_pMotionController,&MotionController::sig_dataChanged,this,&GongYTabWidget::slot_DataChanged);

    updateSpinBoxValue();

    m_pTimer = new QTimer();
    connect(m_pTimer,&QTimer::timeout,this,&GongYTabWidget::slotXrayWarning);

    m_slash=false;
}

GongYTabWidget::~GongYTabWidget()
{
    if(m_pMotionController)
    {
        m_pMotionController->deleteLater();
        m_pMotionController = nullptr;
    }

    if(m_pXraySource)
    {
        XraySource::destroyInstance();
        m_pXraySource=nullptr;
    }

    if(m_pLabel)
    {
        delete m_pLabel;
        m_pLabel=nullptr;
    }
}

float GongYTabWidget::opacity() const
{
    return m_opacity;
}

void GongYTabWidget::setOpacity(float value)
{
    m_opacity = value;
    update();
}

void GongYTabWidget::setSlashMark(bool slashMark)
{
    m_slash = slashMark;
    if(m_slash)
    {
        if(!m_pTimer->isActive())
            m_pTimer->start(150);
        else
            return;
    }
    else
        m_pTimer->stop();

}

void GongYTabWidget::onCurrentChanged(int index)
{
    qDebug() << "tab changed to index:" << index;


    // 获取上一个和当前页签的widget
    QWidget *oldwidget = widget(index - 1 >= 0 ? index - 1 : count() - 1);
    QWidget *newwidget = widget(index);

}

void GongYTabWidget::setupTabs()
{
    // 设置tab位置
    setTabPosition(QTabWidget::North);

    // 设置tab形状
    setTabShape(QTabWidget::Rounded);

    // 添加页签
    addTab(createFirstTab(), tr("扫描设备"));
    addTab(createSecondTab(), tr("图像查看"));

    // 设置默认页签
    setCurrentIndex(0);
}

QWidget *GongYTabWidget::createFirstTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);

    createXrayGroup(layout);
    createDetectorGroup(layout);
    createAcquireGroup(layout);

    tab->setLayout(layout);
    return tab;
}

QWidget *GongYTabWidget::createSecondTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);

    // // 添加各种控件，演示网格布局
    // qlabel *label1 = new qlabel(tr("用户名:"), tab);
    // qlineedit *usernameedit = new qlineedit(tab);
    // usernameedit->setplaceholdertext(tr("请输入用户名"));

    // qlabel *label2 = new qlabel(tr("密码:"), tab);
    // qlineedit *passwordedit = new qlineedit(tab);
    // passwordedit->setechomode(qlineedit::password);
    // passwordedit->setplaceholdertext(tr("请输入密码"));

    // qlabel *label3 = new qlabel(tr("年龄:"), tab);
    // qspinbox *ageedit = new qspinbox(tab);
    // ageedit->setrange(0, 150);

    // qlabel *label4 = new qlabel(tr("性别:"), tab);
    // qcombobox *gendercombo = new qcombobox(tab);
    // gendercombo->additems(qstringlist() << tr("男") << tr("女") << tr("其他"));

    // qlabel *label5 = new qlabel(tr("兴趣爱好:"), tab);
    // qcheckbox *hobby1 = new qcheckbox(tr("阅读"), tab);
    // qcheckbox *hobby2 = new qcheckbox(tr("运动"), tab);
    // qcheckbox *hobby3 = new qcheckbox(tr("音乐"), tab);

    // qpushbutton *submitbutton = new qpushbutton(tr("提交"), tab);
    // qpushbutton *resetbutton = new qpushbutton(tr("重置"), tab);

    // // 添加标签
    // qlabel *titlelabel = new qlabel(tr("用户信息表单"), tab);
    // titlelabel->setstylesheet("font-size: 16px; font-weight: bold; color: #2980b9;");

    // // 布局设置
    // layout->addwidget(titlelabel, 0, 0, 1, 2, qt::aligncenter);
    // layout->addwidget(label1, 1, 0);
    // layout->addwidget(usernameedit, 1, 1);
    // layout->addwidget(label2, 2, 0);
    // layout->addwidget(passwordedit, 2, 1);
    // layout->addwidget(label3, 3, 0);
    // layout->addwidget(ageedit, 3, 1);
    // layout->addwidget(label4, 4, 0);
    // layout->addwidget(gendercombo, 4, 1);
    // layout->addwidget(label5, 5, 0);

    // qhboxlayout *hobbylayout = new qhboxlayout();
    // hobbylayout->addwidget(hobby1);
    // hobbylayout->addwidget(hobby2);
    // hobbylayout->addwidget(hobby3);
    // layout->addlayout(hobbylayout, 5, 1);

    // qhboxlayout *buttonlayout = new qhboxlayout();
    // buttonlayout->addstretch();
    // buttonlayout->addwidget(submitbutton);
    // buttonlayout->addwidget(resetbutton);
    // layout->addlayout(buttonlayout, 6, 0, 1, 2);

    // // 设置间距
    // layout->setspacing(15);
    // layout->setcontentsmargins(20, 20, 20, 20);

    tab->setLayout(layout);
    return tab;
}

void GongYTabWidget::createXrayGroup(QVBoxLayout* layout)
{
    // IO status list section
    QGroupBox* pXrayGroup = new QGroupBox(tr("射线机"));
    pXrayGroup->setFont(m_font);
    pXrayGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px;}");

    QVBoxLayout* pXrayLayout = new QVBoxLayout(pXrayGroup);

    m_pLabel = new QLabel(tr("Hamamatsu"));
    m_pLabel->setFixedHeight(25);
    m_pLabel->setAlignment(Qt::AlignCenter);
    m_pLabel->setFont(m_font);
    m_pLabel->setStyleSheet("border: none; background-color: rgb(255, 0, 0);");
    pXrayLayout->addWidget(m_pLabel);

    QHBoxLayout* pVolCurrentHLayout = new QHBoxLayout();
    QGroupBox* pVoltageGroup = new QGroupBox(tr("Voltage(kV)"));
    pVoltageGroup->setFont(m_font);
    pVoltageGroup->setMaximumHeight(100);
    pVoltageGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px;}");
    QFormLayout* pvolFormLayout = new QFormLayout();
    m_pVolCurrentLineEdit = new QLineEdit();
    m_pVolCurrentLineEdit->setFont(m_font);
    m_pVolCurrentLineEdit->setEnabled(false);
    m_pVolTargetSpinbox = new GyIntSpinBox();
    m_pVolTargetSpinbox->setRange(0,500);
    m_pVolTargetSpinbox->setFont(m_font);
    connect(m_pVolTargetSpinbox->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotVoltageChanged);
    connect(m_pVolTargetSpinbox->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotVoltageChanged);
    pvolFormLayout->addRow(tr("Current:"),m_pVolCurrentLineEdit);
    pvolFormLayout->addRow(tr("Target:"),m_pVolTargetSpinbox);
    pVoltageGroup->setLayout(pvolFormLayout);
    pVolCurrentHLayout->addWidget(pVoltageGroup);

    QGroupBox* pCurrentGroup = new QGroupBox(tr("Current(uA)"));
    pCurrentGroup->setFont(m_font);
    pCurrentGroup->setMaximumHeight(100);
    pCurrentGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px;}");
    QFormLayout* pcurrentFormLayout = new QFormLayout();
    m_pCurrentCurrentLineEdit = new QLineEdit();
    m_pCurrentCurrentLineEdit->setFont(m_font);
    m_pCurrentCurrentLineEdit->setEnabled(false);
    m_pCurrentTargetSpinbox = new GyIntSpinBox();
    m_pCurrentTargetSpinbox->setRange(0,1000);
    m_pCurrentTargetSpinbox->setFont(m_font);
    connect(m_pCurrentTargetSpinbox->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotCurrentChanged);
    connect(m_pCurrentTargetSpinbox->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotCurrentChanged);
    pcurrentFormLayout->addRow(tr("Current"),m_pCurrentCurrentLineEdit);
    pcurrentFormLayout->addRow(tr("Target"),m_pCurrentTargetSpinbox);
    pCurrentGroup->setLayout(pcurrentFormLayout);
    pVolCurrentHLayout->addWidget(pCurrentGroup);

    pXrayLayout->addLayout(pVolCurrentHLayout);
    // 添加按钮
    QHBoxLayout* pBtnsLayout = new QHBoxLayout();
    pBtnsLayout->addStretch();
    m_pButtonXon = new QPushButton(tr("XON"));
    connect(m_pButtonXon,&QPushButton::clicked,this,&GongYTabWidget::slotXonClick);
    pBtnsLayout->addWidget(m_pButtonXon);
    pBtnsLayout->addStretch();
    m_pButtonXoff = new QPushButton(tr("XOFF"));
    connect(m_pButtonXoff,&QPushButton::clicked,this,&GongYTabWidget::slotXoffClick);
    pBtnsLayout->addWidget(m_pButtonXoff);
    pBtnsLayout->addStretch();
    pXrayLayout->addLayout(pBtnsLayout);

    //射线源状态
    QGroupBox* pChecksGroup = new QGroupBox();
    pChecksGroup->setFont(m_font);
    pChecksGroup->setMaximumHeight(50);
    pChecksGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QHBoxLayout* pStatesLayout = new QHBoxLayout();
    m_pXray = new QCheckBox(tr("XRay"));
    m_pXray->setEnabled(false);
    m_pWarmUp = new QCheckBox(tr("Warm Up"));
    m_pWarmUp->setEnabled(false);
    m_pInterLock = new QCheckBox(tr("Inter lock"));
    m_pInterLock->setEnabled(false);
    m_pConnect = new QCheckBox(tr("Connect"));
    m_pConnect->setEnabled(false);
    pStatesLayout->addWidget(m_pXray);
    pStatesLayout->addWidget(m_pWarmUp);
    pStatesLayout->addWidget(m_pInterLock);
    pStatesLayout->addWidget(m_pConnect);
    pChecksGroup->setLayout(pStatesLayout);
    pXrayLayout->addWidget(pChecksGroup);

    layout->addWidget(pXrayGroup);
}

void GongYTabWidget::createDetectorGroup(QVBoxLayout* layout)
{
    QGroupBox* pMotionCtrlGroup = new QGroupBox(tr("运动控制"));
    pMotionCtrlGroup->setFont(m_font);
    pMotionCtrlGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");

    QGridLayout* pGridLayout = new QGridLayout(pMotionCtrlGroup);
    //标题
    QLabel* pXrayCurrentLable = new QLabel(tr("当前值"));
    pXrayCurrentLable->setAlignment(Qt::AlignCenter);
    pXrayCurrentLable->setFont(m_font);
    QLabel* pXrayTargetLable = new QLabel(tr("目标值"));
    pXrayTargetLable->setAlignment(Qt::AlignCenter);
    pXrayTargetLable->setFont(m_font);
    QLabel* pXraySpeedLable = new QLabel(tr("速度"));
    pXraySpeedLable->setAlignment(Qt::AlignCenter);
    pXraySpeedLable->setFont(m_font);
    QPushButton* pXrayStopAllBtn = new QPushButton(tr("Stop all"));
    connect(pXrayStopAllBtn,&QPushButton::clicked,this,&GongYTabWidget::slotStopAllBtnClick);
    pXrayStopAllBtn->setFont(m_font);
    pGridLayout->addWidget(pXrayCurrentLable,0,1,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayTargetLable,0,2,Qt::AlignCenter);
    pGridLayout->addWidget(pXraySpeedLable,0,3,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayStopAllBtn,0,4,1,4,Qt::AlignCenter);

    //画线
    QLabel* pXrayLineLable = new QLabel(tr("射线源"));
    pXrayLineLable->setFont(m_font);
    GyHorizontalLine* pXrayHoriLineLabel = new GyHorizontalLine();
    pXrayHoriLineLabel->setFont(m_font);
    pGridLayout->addWidget(pXrayLineLable,1,0,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayHoriLineLabel,1,1,1,7);

    //画x轴
    QLabel* pXrayXAixs = new QLabel(tr("X轴 (mm)"));
    pXrayXAixs->setFont(m_font);
    m_pXrayCurrentPos = new GyDoubleSpinBox(1,Xray_X);
    m_pXrayCurrentPos->setFont(m_font);
    m_pXrayTargetPos = new GyDoubleSpinBox(2,Xray_X);
    m_pXrayTargetPos->setRange(-100,1000);
    connect(m_pXrayTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pXrayTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pXrayTargetPos->setFont(m_font);
    m_pXraySpeed = new GyDoubleSpinBox(3,Xray_X);
    connect(m_pXraySpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    // connect(m_pXraySpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    m_pXraySpeed->setFont(m_font);
    GyPushButton* pXrayXLeftBtn = new GyPushButton(Xray_X,tr("向左"));/*向左*/
    // pXrayXLeftBtn->setIcon(QIcon(":/device\\img/motionleft"));
    connect(pXrayXLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pXrayXLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pXrayXLeftBtn->setFont(m_font);
    GyPushButton* pXrayXRightBtn = new GyPushButton(Xray_X,tr("向右"));//向右
    // pXrayXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pXrayXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pXrayXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pXrayXRightBtn->setFont(m_font);
    GyPushButton* pXrayXHomeBtn = new GyPushButton(Xray_X,tr("回零"));//
    //pXrayXHomeBtn->setIcon(QIcon(":/img/home.png"));
    connect(pXrayXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pXrayXHomeBtn->setFont(m_font);
    GyPushButton* pXrayStopBtn = new GyPushButton(Xray_X, tr("停止"));//停止
    // pXrayStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pXrayStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pXrayStopBtn->setFont(m_font);
    pGridLayout->addWidget(pXrayXAixs,2,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayCurrentPos,2,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayTargetPos,2,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXraySpeed,2,3,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayXLeftBtn,2,4,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayXRightBtn,2,5,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayXHomeBtn,2,6,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayStopBtn,2,7,Qt::AlignCenter);

    //画z轴
    QLabel* pXrayZAixs = new QLabel(tr("Z轴 (mm)"));
    pXrayZAixs->setFont(m_font);
    m_pXrayZCurrentPos = new GyDoubleSpinBox(4,Xray_Z);
    m_pXrayZCurrentPos->setFont(m_font);
    m_pXrayZTargetPos = new GyDoubleSpinBox(5,Xray_Z);
    m_pXrayZTargetPos->setFont(m_font);
    m_pXrayZTargetPos->setRange(-100,1000);
    connect(m_pXrayZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pXrayZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pXrayZSpeed = new GyDoubleSpinBox(6,Xray_Z);
    m_pXrayZSpeed->setFont(m_font);
    connect(m_pXrayZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    // connect(m_pXrayZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pXrayZLeftBtn = new GyPushButton(Xray_Z,tr("向左"));
    pXrayZLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pXrayZLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pXrayZLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pXrayZLeftBtn->setFont(m_font);
    GyPushButton* pXrayZXRightBtn = new GyPushButton(Xray_Z,tr("向右"));
    pXrayZXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pXrayZXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pXrayZXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pXrayZXRightBtn->setFont(m_font);
    GyPushButton* pXrayZXHomeBtn = new GyPushButton(Xray_Z,tr("回零"));
    connect(pXrayZXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pXrayZXHomeBtn->setFont(m_font);

    GyPushButton* pXrayZStopBtn = new GyPushButton(Xray_Z,tr("停止"));
    pXrayZStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pXrayZStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pXrayZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pXrayZAixs,3,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZCurrentPos,3,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZTargetPos,3,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZSpeed,3,3,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZLeftBtn,3,4,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZXRightBtn,3,5,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZXHomeBtn,3,6,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZStopBtn,3,7,Qt::AlignCenter);

    //载物台
    QLabel* pStageLabel = new QLabel(tr("载物台"));
    pStageLabel->setFont(m_font);
    GyHorizontalLine* pStageHoriLine = new GyHorizontalLine();
    pStageHoriLine->setFont(m_font);
    pGridLayout->addWidget(pStageLabel,4,0,Qt::AlignCenter);
    pGridLayout->addWidget(pStageHoriLine,4,1,1,7);

    //画x轴
    QLabel* pStageXAixs = new QLabel(tr("X轴 (mm)"));
    pStageXAixs->setFont(m_font);
    m_pStageCurrentPos = new GyDoubleSpinBox(7,Material_X);
    m_pStageCurrentPos->setFont(m_font);
    m_pStageTargetPos = new GyDoubleSpinBox(8,Material_X);
    m_pStageTargetPos->setFont(m_font);
    m_pStageTargetPos->setRange(-100,1000);
    connect(m_pStageTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pStageTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageSpeed = new GyDoubleSpinBox(9,Material_X);
    m_pStageSpeed->setFont(m_font);
    connect(m_pStageSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    // connect(m_pStageSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pStageXLeftBtn = new GyPushButton(Material_X,tr("向左"));
    pStageXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pStageXLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pStageXLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pStageXLeftBtn->setFont(m_font);
    GyPushButton* pStageXRightBtn = new GyPushButton(Material_X,tr("向右"));
    pStageXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pStageXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pStageXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pStageXRightBtn->setFont(m_font);
    GyPushButton* pStageXHomeBtn = new GyPushButton(Material_X,tr("回零"));
    connect(pStageXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pStageXHomeBtn->setFont(m_font);

    GyPushButton* pStageStopBtn = new GyPushButton(Material_X,tr("停止"));
    pStageStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pStageStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pStageStopBtn->setFont(m_font);
    pGridLayout->addWidget(pStageXAixs,5,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageCurrentPos,5,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageTargetPos,5,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageSpeed,5,3,Qt::AlignCenter);
    pGridLayout->addWidget(pStageXLeftBtn,5,4,Qt::AlignCenter);
     pGridLayout->addWidget(pStageXRightBtn,5,5,Qt::AlignCenter);
     pGridLayout->addWidget(pStageXHomeBtn,5,6,Qt::AlignCenter);
    pGridLayout->addWidget(pStageStopBtn,5,7,Qt::AlignCenter);

    //画y轴
    QLabel* pStageYAixs = new QLabel(tr("Y轴 (mm)"));
    pStageYAixs->setFont(m_font);
    m_pStageYCurrentPos = new GyDoubleSpinBox(10,Material_Y);
    m_pStageYCurrentPos->setFont(m_font);
    m_pStageYTargetPos = new GyDoubleSpinBox(11,Material_Y);
    m_pStageYTargetPos->setFont(m_font);
    m_pStageYTargetPos->setRange(-100,1000);
    connect(m_pStageYTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pStageYTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageYSpeed = new GyDoubleSpinBox(12,Material_Y);
    m_pStageYSpeed->setFont(m_font);
    connect(m_pStageYSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    // connect(m_pStageYSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pStageYXLeftBtn = new GyPushButton(Material_Y,tr("向左"));
    pStageYXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pStageYXLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pStageYXLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pStageYXLeftBtn->setFont(m_font);
    GyPushButton* pStageYXRightBtn = new GyPushButton(Material_Y,tr("向右"));
    pStageYXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pStageYXRightBtn,&GyPushButton::pressed,this,&::GongYTabWidget::slotRightButtonPress);
    connect(pStageYXRightBtn,&GyPushButton::released,this,&::GongYTabWidget::slotButtonRelease);
    pStageYXRightBtn->setFont(m_font);
    GyPushButton* pStageYXHomeBtn = new GyPushButton(Material_Y,tr("回零"));
    connect(pStageYXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pStageYXHomeBtn->setFont(m_font);

    GyPushButton* pStageYStopBtn = new GyPushButton(Material_Y,tr("停止"));
    pStageYStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pStageYStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pStageYStopBtn->setFont(m_font);
    pGridLayout->addWidget(pStageYAixs,6,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageYCurrentPos,6,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageYTargetPos,6,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageYSpeed,6,3,Qt::AlignCenter);
    pGridLayout->addWidget(pStageYXLeftBtn,6,4,Qt::AlignCenter);
    pGridLayout->addWidget(pStageYXRightBtn,6,5,Qt::AlignCenter);
    pGridLayout->addWidget(pStageYXHomeBtn,6,6,Qt::AlignCenter);
    pGridLayout->addWidget(pStageYStopBtn,6,7,Qt::AlignCenter);
    //画z轴
    QLabel* pStageZAixs = new QLabel(tr("Z轴 (mm)"));
    pStageZAixs->setFont(m_font);
    m_pStageZCurrentPos = new GyDoubleSpinBox(13,Material_Z);
    m_pStageZCurrentPos->setFont(m_font);
    m_pStageZTargetPos = new GyDoubleSpinBox(14,Material_Z);
    m_pStageZTargetPos->setFont(m_font);
    m_pStageZTargetPos->setRange(-100,1000);
    connect(m_pStageZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pStageZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageZSpeed = new GyDoubleSpinBox(15,Material_Z);
    m_pStageZSpeed->setFont(m_font);
    connect(m_pStageZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    // connect(m_pStageZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pStageZLeftBtn = new GyPushButton(Material_Z,tr("向左"));
    pStageZLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pStageZLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pStageZLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pStageZLeftBtn->setFont(m_font);
    GyPushButton* pStageZXRightBtn = new GyPushButton(Material_Z,tr("向右"));
    pStageZXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pStageZXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pStageZXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pStageZXRightBtn->setFont(m_font);
    GyPushButton* pStageZXHomeBtn = new GyPushButton(Material_Z,tr("回零"));
    connect(pStageZXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pStageZXHomeBtn->setFont(m_font);

    GyPushButton* pStageZStopBtn = new GyPushButton(Material_Z,tr("停止"));
    pStageZStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pStageZStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pStageZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pStageZAixs,7,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageZCurrentPos,7,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageZTargetPos,7,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageZSpeed,7,3,Qt::AlignCenter);
    pGridLayout->addWidget(pStageZLeftBtn,7,4,Qt::AlignCenter);
    pGridLayout->addWidget(pStageZXRightBtn,7,5,Qt::AlignCenter);
    pGridLayout->addWidget(pStageZXHomeBtn,7,6,Qt::AlignCenter);
    pGridLayout->addWidget(pStageZStopBtn,7,7,Qt::AlignCenter);


    //探测器
    QLabel* pDetectorLabel = new QLabel(tr("探测器"));
    pDetectorLabel->setFont(m_font);
    GyHorizontalLine* pDetectorLine = new GyHorizontalLine();
    pDetectorLine->setFont(m_font);
    pGridLayout->addWidget(pDetectorLabel,8,0,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorLine,8,1,1,7);

    //画x轴
    QLabel* pDetectorXAixs = new QLabel(tr("X轴 (mm)"));
    pDetectorXAixs->setFont(m_font);
    m_pDetectorCurrentPos = new GyDoubleSpinBox(16,Detector_X);
    m_pDetectorCurrentPos->setFont(m_font);
    m_pDetectorTargetPos = new GyDoubleSpinBox(17,Detector_X);
    m_pDetectorTargetPos->setFont(m_font);
    m_pDetectorTargetPos->setRange(-100,1000);
    connect(m_pDetectorTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pDetectorTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorSpeed = new GyDoubleSpinBox(18,Detector_X);
    m_pDetectorSpeed->setFont(m_font);
    connect(m_pDetectorSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    // connect(m_pDetectorSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pDetectorXLeftBtn = new GyPushButton(Detector_X,tr("向左"));
    pDetectorXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pDetectorXLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pDetectorXLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pDetectorXLeftBtn->setFont(m_font);
    GyPushButton* pDetectorXRightBtn = new GyPushButton(Detector_X,tr("向右"));
    pDetectorXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pDetectorXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pDetectorXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pDetectorXRightBtn->setFont(m_font);
    GyPushButton* pDetectorXHomeBtn = new GyPushButton(Detector_X,tr("回零"));
    connect(pDetectorXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pDetectorXHomeBtn->setFont(m_font);
    GyPushButton* pDetectorStopBtn = new GyPushButton(Detector_X,tr("停止"));
    pDetectorStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pDetectorStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pDetectorStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorXAixs,9,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorCurrentPos,9,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorTargetPos,9,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorSpeed,9,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorXLeftBtn,9,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorXRightBtn,9,5,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorXHomeBtn,9,6,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorStopBtn,9,7,Qt::AlignCenter);

    //画y轴
    QLabel* pDetectorYAixs = new QLabel(tr("Y轴 (mm)"));
    pDetectorYAixs->setFont(m_font);
    m_pDetectorYCurrentPos = new GyDoubleSpinBox(19,Detector_Y);
    m_pDetectorYCurrentPos->setFont(m_font);
    m_pDetectorYTargetPos = new GyDoubleSpinBox(20,Detector_Y);
    m_pDetectorYTargetPos->setFont(m_font);
    m_pDetectorYTargetPos->setRange(-100,1000);
    connect(m_pDetectorYTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pDetectorYTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorYSpeed = new GyDoubleSpinBox(21,Detector_Y);
    m_pDetectorYSpeed->setFont(m_font);
    connect(m_pDetectorYSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pDetectorYSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    GyPushButton* pDetectorYXLeftBtn = new GyPushButton(Detector_Y,tr("向左"));
    pDetectorYXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pDetectorYXLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pDetectorYXLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pDetectorYXLeftBtn->setFont(m_font);
    GyPushButton* pDetectorYXRightBtn = new GyPushButton(Detector_Y,tr("向右"));
    pDetectorYXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pDetectorYXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pDetectorYXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pDetectorYXRightBtn->setFont(m_font);
    GyPushButton* pDetectorYXHomeBtn = new GyPushButton(Detector_Y,tr("回零"));
    connect(pDetectorYXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pDetectorYXHomeBtn->setFont(m_font);

    GyPushButton* pDetectorYStopBtn = new GyPushButton(Detector_Y,tr("停止"));
    pDetectorYStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pDetectorYStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pDetectorYStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorYAixs,10,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorYCurrentPos,10,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorYTargetPos,10,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorYSpeed,10,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorYXLeftBtn,10,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorYXRightBtn,10,5,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorYXHomeBtn,10,6,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorYStopBtn,10,7,Qt::AlignCenter);
    //画z轴
    QLabel* pDetectorZAixs = new QLabel(tr("Z轴 (mm)"));
    pDetectorZAixs->setFont(m_font);
    m_pDetectorZCurrentPos = new GyDoubleSpinBox(22,Detector_Z);
    m_pDetectorZCurrentPos->setFont(m_font);
    m_pDetectorZTargetPos = new GyDoubleSpinBox(23,Detector_Z);
    m_pDetectorZTargetPos->setFont(m_font);
    m_pDetectorZTargetPos->setRange(-100,1000);
    connect(m_pDetectorZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pDetectorZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorZSpeed = new GyDoubleSpinBox(24,Detector_Z);
    m_pDetectorZSpeed->setFont(m_font);
    connect(m_pDetectorZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    // connect(m_pDetectorZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    GyPushButton* pDetectorZXLeftBtn = new GyPushButton(Detector_Z,tr("向左"));
    pDetectorZXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pDetectorZXLeftBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotLeftButtonPress);
    connect(pDetectorZXLeftBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pDetectorZXLeftBtn->setFont(m_font);
    GyPushButton* pDetectorZXRightBtn = new GyPushButton(Detector_Z,tr("向右"));
    pDetectorZXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pDetectorZXRightBtn,&GyPushButton::pressed,this,&GongYTabWidget::slotRightButtonPress);
    connect(pDetectorZXRightBtn,&GyPushButton::released,this,&GongYTabWidget::slotButtonRelease);
    pDetectorZXRightBtn->setFont(m_font);
    GyPushButton* pDetectorZXHomeBtn = new GyPushButton(Detector_Z,tr("回零"));
    connect(pDetectorZXHomeBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotHomeButtonClick);
    pDetectorZXHomeBtn->setFont(m_font);
    GyPushButton* pDetectorZStopBtn = new GyPushButton(Detector_Z,tr("停止"));
    pDetectorZStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pDetectorZStopBtn,&GyPushButton::clicked,this,&GongYTabWidget::slotEndButtonClick);
    pDetectorZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorZAixs,11,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZCurrentPos,11,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZTargetPos,11,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZSpeed,11,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXLeftBtn,11,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXRightBtn,11,5,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXHomeBtn,11,6,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZStopBtn,11,7,Qt::AlignCenter);


    layout->addWidget(pMotionCtrlGroup);
}

void GongYTabWidget::createAcquireGroup(QVBoxLayout* pGroupLayout)
{
    QGroupBox* pAcquireGroup = new QGroupBox(tr("采集图片"));
    pAcquireGroup->setFont(m_font);
    pAcquireGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");

    QVBoxLayout* pAcquireVLayout = new QVBoxLayout(pAcquireGroup);
    QHBoxLayout* pAcquireModeHLayout = new QHBoxLayout();
    QLabel* pModelLabel = new QLabel(tr("采集模式"));
    QRadioButton* pStepRadioBtn = new QRadioButton(tr("步进"));
    QRadioButton* pContineRadioBtn = new QRadioButton(tr("连续"));
    QRadioButton* pDrRadioBtn = new QRadioButton(tr("DR"));
    QRadioButton* pSpiralRadioBtn = new QRadioButton(tr("螺旋"));
    QRadioButton* pBiasRadioBtn = new QRadioButton(tr("偏置"));
    pAcquireModeHLayout->addWidget(pModelLabel);
    pAcquireModeHLayout->addWidget(pStepRadioBtn);
    pAcquireModeHLayout->addWidget(pContineRadioBtn);
    pAcquireModeHLayout->addWidget(pDrRadioBtn);
    pAcquireModeHLayout->addWidget(pSpiralRadioBtn);
    pAcquireModeHLayout->addWidget(pBiasRadioBtn);

    pAcquireVLayout->addLayout(pAcquireModeHLayout);
    pGroupLayout->addWidget(pAcquireGroup);
}

void GongYTabWidget::updateSpinBoxValue()
{
    m_pXraySpeed->setValue(MotionConfig::lastAxisSpeed[Xray_X]);
    m_pXrayTargetPos->setValue(MotionConfig::lastAxisPos[Xray_X]);
    m_pXrayZTargetPos->setValue(MotionConfig::lastAxisPos[Xray_Z]);
    m_pXrayZSpeed->setValue(MotionConfig::lastAxisSpeed[Xray_Z]);

    m_pStageSpeed->setValue(MotionConfig::lastAxisSpeed[Material_X]);
    m_pStageTargetPos->setValue(MotionConfig::lastAxisPos[Material_X]);
    m_pStageYTargetPos->setValue(MotionConfig::lastAxisPos[Material_Y]);
    m_pStageYSpeed->setValue(MotionConfig::lastAxisSpeed[Material_Y]);
    m_pStageZTargetPos->setValue(MotionConfig::lastAxisPos[Material_Z]);
    m_pStageZSpeed->setValue(MotionConfig::lastAxisSpeed[Material_Z]);

    m_pDetectorSpeed->setValue(MotionConfig::lastAxisSpeed[Detector_X]);
    m_pDetectorTargetPos->setValue(MotionConfig::lastAxisPos[Detector_X]);
    m_pDetectorYTargetPos->setValue(MotionConfig::lastAxisPos[Detector_Y]);
    m_pDetectorYSpeed->setValue(MotionConfig::lastAxisSpeed[Detector_Y]);
    m_pDetectorZTargetPos->setValue(MotionConfig::lastAxisPos[Detector_Z]);
    m_pDetectorZSpeed->setValue(MotionConfig::lastAxisSpeed[Detector_Z]);

    MotionConfig::writeMotionConfig();
}

void GongYTabWidget::slotXrayWarning()
{
    QString currentStyle = m_pLabel->styleSheet();
    if (currentStyle.contains("red")) {
        m_pLabel->setStyleSheet("background-color: white;");
    } else {
        m_pLabel->setStyleSheet("background-color: red;");
    }
}

void GongYTabWidget::setCheckboxStyle(QCheckBox* pCheckbox)
{
    pCheckbox->setStyleSheet(R"(
        /* 未选中状态：复选框边框和内部指示器为红色 */
        QCheckBox::indicator:unchecked {
            border: 2px solid red;       /* 边框颜色 */
            background-color: red;     /* 背景色 */
            width: 18px;                 /* 复选框宽度 */
            height: 18px;                /* 复选框高度 */
            border-radius: 3px;          /* 圆角（可选） */
        }

        /* 选中状态：复选框内部对勾和背景为绿色 */
        QCheckBox::indicator:checked {
            border: 2px solid green;     /* 边框颜色 */
            background-color: green;     /* 背景色 */
            width: 18px;
            height: 18px;
            border-radius: 3px;
        }

        /* 选中状态：对勾颜色（默认白色，可调整） */
        QCheckBox::indicator:checked::indicator {
            color: white;                /* 对勾颜色 */
        }
    )");
}

void GongYTabWidget::slotXonClick()
{
    if(!m_online)
        return;

    m_pXraySource->setXrayOn();
}

void GongYTabWidget::slotXoffClick()
{
    if(!m_online)
        return;

    m_pXraySource->setXrayOff();
}

void GongYTabWidget::slotVoltageChanged()
{
    if(m_online && m_pXraySource)
    {
        m_pXraySource->setXrayKv(m_pVolTargetSpinbox->value());
    }
}

void GongYTabWidget::slotCurrentChanged()
{
    if(m_online && m_pXraySource)
    {
        m_pXraySource->setXrayBeam(m_pCurrentTargetSpinbox->value());
    }
}

void GongYTabWidget::slotAxisPosChanged()
{
    QLineEdit* pSendBox = qobject_cast<QLineEdit*>(sender());
    if(pSendBox == nullptr)
        return;

    GyDoubleSpinBox* parent = qobject_cast<GyDoubleSpinBox*>(pSendBox->parentWidget());
    if(parent == nullptr)
        return;

    AXIS axis = parent->Axis();
    float value = parent->value();
    // ui->textEdit->append(QString(u8"轴%1 %2 设置目标位置 %3").arg(ui->comboBox->currentIndex()).arg(ui->comboBox->currentText()).arg(ui->edit_pos->text()));
    MotionConfig::lastAxisPos[axis] = value;
    m_pMotionController->onBtn_axisStartClicked(axis,QString::number(value));
}

void GongYTabWidget::slotAxisSpeedChanged()
{
    QLineEdit* pSendBox = qobject_cast<QLineEdit*>(sender());
    if(pSendBox == nullptr)
        return;

    GyDoubleSpinBox* parentSpinBox = qobject_cast<GyDoubleSpinBox*>(pSendBox->parentWidget());
    if(parentSpinBox==nullptr)
        return;
    AXIS curAxis = parentSpinBox->Axis();
    float value = parentSpinBox->value();
    if(curAxis == /*annulus_W*/4)
    {
        if(value>40)
        {
            MotionConfig::lastAxisSpeed[curAxis] =  40;
        }else
        {
            MotionConfig::lastAxisSpeed[curAxis] =  value;
        }
    }
    else
    {
        MotionConfig::lastAxisSpeed[curAxis] = value;
    }

}

void GongYTabWidget::slot_DataChanged(const QVariant &var)
{
    QVector<MotionCtrlData::MotionCtrlInfo> mcVec = var.value<QVector<MotionCtrlData::MotionCtrlInfo>>();

    //运动控制，载物台x轴
    m_pXrayCurrentPos->setValue(mcVec[Xray_X].axEncPos);
    m_pXrayZCurrentPos->setValue(mcVec[Xray_Z].axEncPos);

    m_pStageCurrentPos->setValue(mcVec[Material_X].axEncPos);
    m_pStageYCurrentPos->setValue(mcVec[Material_Y].axEncPos);
    m_pStageZCurrentPos->setValue(mcVec[Material_Z].axEncPos);

    m_pDetectorCurrentPos->setValue(mcVec[Detector_X].axEncPos);
    m_pDetectorYCurrentPos->setValue(mcVec[Detector_Y].axEncPos);
    m_pDetectorZCurrentPos->setValue(mcVec[Detector_Z].axEncPos);
}

void GongYTabWidget::slotStartButtonClick()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    double value=0.0;


    QList<GyDoubleSpinBox*> spinBoxList;
    spinBoxList << m_pXrayTargetPos << m_pXrayZTargetPos << m_pStageTargetPos<<m_pStageYTargetPos<<m_pStageZTargetPos<<m_pDetectorTargetPos<<m_pDetectorYTargetPos<<m_pDetectorZTargetPos;
    foreach (GyDoubleSpinBox* item, spinBoxList) {
        if(item->Axis() == axis)
        {
            value = item->value();
            break;
        }
    }
    qDebug()<<value;
    m_pMotionController->onBtn_axisStartClicked(axis,QString::number(value));
}

void GongYTabWidget::slotEndButtonClick()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisStopClicked(axis);
}

void GongYTabWidget::slotLeftButtonPress()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisLeftPressed(axis);
}

void GongYTabWidget::slotRightButtonPress()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisRightPressed(axis);
}

void GongYTabWidget::slotButtonRelease()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisStopClicked(axis);
}

void GongYTabWidget::slotHomeButtonClick()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisResetPosClicked(axis);
}

void GongYTabWidget::slotStopAllBtnClick()
{
    if(m_online && m_pMotionController)
    {
        m_pMotionController->onBtn_axisStopClicked(Xray_X);
        m_pMotionController->onBtn_axisStopClicked(Xray_Z);
        m_pMotionController->onBtn_axisStopClicked(Material_X);
        m_pMotionController->onBtn_axisStopClicked(Material_Y);
        m_pMotionController->onBtn_axisStopClicked(Material_Z);
        m_pMotionController->onBtn_axisStopClicked(Detector_X);
        m_pMotionController->onBtn_axisStopClicked(Detector_Y);
        m_pMotionController->onBtn_axisStopClicked(Detector_Z);
    }
}

void GongYTabWidget::slotGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua)
{
    m_pVolCurrentLineEdit->setText(QString::number(kv));
    m_pCurrentCurrentLineEdit->setText(QString::number(ua));

    m_online=true;

    if (locked) {
        m_pInterLock->setChecked(true);
        setCheckboxStyle(m_pInterLock);
        m_pWarmUp->setChecked(false);
        setCheckboxStyle(m_pWarmUp);
        m_pXray->setChecked(false);
        setCheckboxStyle(m_pXray);
        m_pConnect->setChecked(false);
        setCheckboxStyle(m_pConnect);
        setSlashMark(false);
    } else if (warmup) {
        m_pWarmUp->setChecked(true);
        setSlashMark(true);
        setCheckboxStyle(m_pWarmUp);
        m_pInterLock->setChecked(false);
        setCheckboxStyle(m_pInterLock);
        m_pXray->setChecked(false);
        setCheckboxStyle(m_pXray);
        m_pConnect->setChecked(true);
        setCheckboxStyle(m_pConnect);
    } else if (on) {
        m_pXray->setChecked(true);
        setCheckboxStyle(m_pXray);
        m_pInterLock->setChecked(false);
        setCheckboxStyle(m_pInterLock);
        m_pWarmUp->setChecked(false);
        setCheckboxStyle(m_pWarmUp);
        m_pConnect->setChecked(true);
        setCheckboxStyle(m_pConnect);
        setSlashMark(false);
    } else {
        m_pXray->setChecked(false);
        setCheckboxStyle(m_pXray);
        m_pInterLock->setChecked(false);
        setCheckboxStyle(m_pInterLock);
        m_pWarmUp->setChecked(false);
        setCheckboxStyle(m_pWarmUp);
        m_pConnect->setChecked(true);
        setCheckboxStyle(m_pConnect);
        setSlashMark(false);
    }
}
