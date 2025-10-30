#include "motionwidget.h"
#include "xraysource.h"
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


#pragma execution_character_set("utf-8")

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


    m_pTimer = new QTimer();
    connect(m_pTimer,&QTimer::timeout,this,&GongYTabWidget::slotXrayWarning);

    m_slash=false;
}

GongYTabWidget::~GongYTabWidget()
{
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
            m_pTimer->start(50);
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
    pXrayStopAllBtn->setFont(m_font);
    pGridLayout->addWidget(pXrayCurrentLable,0,1,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayTargetLable,0,2,Qt::AlignCenter);
    pGridLayout->addWidget(pXraySpeedLable,0,3,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayStopAllBtn,0,4,1,2,Qt::AlignCenter);

    //画线
    QLabel* pXrayLineLable = new QLabel(tr("射线源"));
    pXrayLineLable->setFont(m_font);
    GyHorizontalLine* pXrayHoriLineLabel = new GyHorizontalLine();
    pXrayHoriLineLabel->setFont(m_font);
    pGridLayout->addWidget(pXrayLineLable,1,0,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayHoriLineLabel,1,1,1,5);

    //画x轴
    QLabel* pXrayXAixs = new QLabel(tr("X轴 (mm)"));
    pXrayXAixs->setFont(m_font);
    m_pXrayCurrentPos = new GyDoubleSpinBox();
    m_pXrayCurrentPos->setFont(m_font);
    m_pXrayTargetPos = new GyDoubleSpinBox();
    connect(m_pXrayTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pXrayTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pXrayTargetPos->setFont(m_font);
    m_pXraySpeed = new GyDoubleSpinBox();
    connect(m_pXraySpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    connect(m_pXraySpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    m_pXraySpeed->setFont(m_font);
    QPushButton* pXrayXStartBtn = new QPushButton(tr("开始"));
    pXrayXStartBtn->setFont(m_font);
    QPushButton* pXrayStopBtn = new QPushButton(tr("停止"));
    pXrayStopBtn->setFont(m_font);
    pGridLayout->addWidget(pXrayXAixs,2,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayCurrentPos,2,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayTargetPos,2,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXraySpeed,2,3,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayXStartBtn,2,4,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayStopBtn,2,5,Qt::AlignCenter);

    //画z轴
    QLabel* pXrayZAixs = new QLabel(tr("Z轴 (mm)"));
    pXrayZAixs->setFont(m_font);
    m_pXrayZCurrentPos = new GyDoubleSpinBox();
    m_pXrayZCurrentPos->setFont(m_font);
    m_pXrayZTargetPos = new GyDoubleSpinBox();
    m_pXrayZTargetPos->setFont(m_font);
    connect(m_pXrayZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pXrayZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pXrayZSpeed = new GyDoubleSpinBox();
    m_pXrayZSpeed->setFont(m_font);
    connect(m_pXrayZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    connect(m_pXrayZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    QPushButton* pXrayZXStartBtn = new QPushButton(tr("开始"));
    pXrayZXStartBtn->setFont(m_font);
    QPushButton* pXrayZStopBtn = new QPushButton(tr("停止"));
    pXrayZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pXrayZAixs,3,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZCurrentPos,3,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZTargetPos,3,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZSpeed,3,3,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZXStartBtn,3,4,Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZStopBtn,3,5,Qt::AlignCenter);

    //载物台
    QLabel* pStageLabel = new QLabel(tr("载物台"));
    pStageLabel->setFont(m_font);
    GyHorizontalLine* pStageHoriLine = new GyHorizontalLine();
    pStageHoriLine->setFont(m_font);
    pGridLayout->addWidget(pStageLabel,4,0,Qt::AlignCenter);
    pGridLayout->addWidget(pStageHoriLine,4,1,1,5);

    //画x轴
    QLabel* pStageXAixs = new QLabel(tr("X轴 (mm)"));
    pStageXAixs->setFont(m_font);
    m_pStageCurrentPos = new GyDoubleSpinBox();
    m_pStageCurrentPos->setFont(m_font);
    m_pStageTargetPos = new GyDoubleSpinBox();
    m_pStageTargetPos->setFont(m_font);
    connect(m_pStageTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pStageTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageSpeed = new GyDoubleSpinBox();
    m_pStageSpeed->setFont(m_font);
    connect(m_pStageSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    connect(m_pStageSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    QPushButton* pStageXStartBtn = new QPushButton(tr("开始"));
    pStageXStartBtn->setFont(m_font);
    QPushButton* pStageStopBtn = new QPushButton(tr("停止"));
    pStageStopBtn->setFont(m_font);
    pGridLayout->addWidget(pStageXAixs,5,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageCurrentPos,5,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageTargetPos,5,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageSpeed,5,3,Qt::AlignCenter);
    pGridLayout->addWidget(pStageXStartBtn,5,4,Qt::AlignCenter);
    pGridLayout->addWidget(pStageStopBtn,5,5,Qt::AlignCenter);

    //画y轴
    QLabel* pStageYAixs = new QLabel(tr("Y轴 (mm)"));
    pStageYAixs->setFont(m_font);
    m_pStageYCurrentPos = new GyDoubleSpinBox();
    m_pStageYCurrentPos->setFont(m_font);
    m_pStageYTargetPos = new GyDoubleSpinBox();
    m_pStageYTargetPos->setFont(m_font);
    connect(m_pStageYTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pStageYTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageYSpeed = new GyDoubleSpinBox();
    m_pStageYSpeed->setFont(m_font);
    connect(m_pStageYSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    connect(m_pStageYSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    QPushButton* pStageYXStartBtn = new QPushButton(tr("开始"));
    pStageYXStartBtn->setFont(m_font);
    QPushButton* pStageYStopBtn = new QPushButton(tr("停止"));
    pStageYStopBtn->setFont(m_font);
    pGridLayout->addWidget(pStageYAixs,6,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageYCurrentPos,6,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageYTargetPos,6,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageYSpeed,6,3,Qt::AlignCenter);
    pGridLayout->addWidget(pStageYXStartBtn,6,4,Qt::AlignCenter);
    pGridLayout->addWidget(pStageYStopBtn,6,5,Qt::AlignCenter);
    //画z轴
    QLabel* pStageZAixs = new QLabel(tr("Z轴 (mm)"));
    pStageZAixs->setFont(m_font);
    m_pStageZCurrentPos = new GyDoubleSpinBox();
    m_pStageZCurrentPos->setFont(m_font);
    m_pStageZTargetPos = new GyDoubleSpinBox();
    m_pStageZTargetPos->setFont(m_font);
    connect(m_pStageZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pStageZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageZSpeed = new GyDoubleSpinBox();
    m_pStageZSpeed->setFont(m_font);
    connect(m_pStageZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    connect(m_pStageZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    QPushButton* pStageZXStartBtn = new QPushButton(tr("开始"));
    pStageZXStartBtn->setFont(m_font);
    QPushButton* pStageZStopBtn = new QPushButton(tr("停止"));
    pStageZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pStageZAixs,7,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageZCurrentPos,7,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageZTargetPos,7,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pStageZSpeed,7,3,Qt::AlignCenter);
    pGridLayout->addWidget(pStageZXStartBtn,7,4,Qt::AlignCenter);
    pGridLayout->addWidget(pStageZStopBtn,7,5,Qt::AlignCenter);


    //探测器
    QLabel* pDetectorLabel = new QLabel(tr("探测器"));
    pDetectorLabel->setFont(m_font);
    GyHorizontalLine* pDetectorLine = new GyHorizontalLine();
    pDetectorLine->setFont(m_font);
    pGridLayout->addWidget(pDetectorLabel,8,0,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorLine,8,1,1,5);

    //画x轴
    QLabel* pDetectorXAixs = new QLabel(tr("X轴 (mm)"));
    pDetectorXAixs->setFont(m_font);
    m_pDetectorCurrentPos = new GyDoubleSpinBox();
    m_pDetectorCurrentPos->setFont(m_font);
    m_pDetectorTargetPos = new GyDoubleSpinBox();
    m_pDetectorTargetPos->setFont(m_font);
    connect(m_pDetectorTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pDetectorTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorSpeed = new GyDoubleSpinBox();
    m_pDetectorSpeed->setFont(m_font);
    connect(m_pDetectorSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisSpeedChanged);
    connect(m_pDetectorSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    QPushButton* pDetectorXStartBtn = new QPushButton(tr("开始"));
    pDetectorXStartBtn->setFont(m_font);
    QPushButton* pDetectorStopBtn = new QPushButton(tr("停止"));
    pDetectorStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorXAixs,9,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorCurrentPos,9,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorTargetPos,9,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorSpeed,9,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorXStartBtn,9,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorStopBtn,9,5,Qt::AlignCenter);

    //画y轴
    QLabel* pDetectorYAixs = new QLabel(tr("Y轴 (mm)"));
    pDetectorYAixs->setFont(m_font);
    m_pDetectorYCurrentPos = new GyDoubleSpinBox();
    m_pDetectorYCurrentPos->setFont(m_font);
    m_pDetectorYTargetPos = new GyDoubleSpinBox();
    m_pDetectorYTargetPos->setFont(m_font);
    connect(m_pDetectorYTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pDetectorYTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorYSpeed = new GyDoubleSpinBox();
    m_pDetectorYSpeed->setFont(m_font);
    connect(m_pDetectorYSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pDetectorYSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    QPushButton* pDetectorYXStartBtn = new QPushButton(tr("开始"));
    pDetectorYXStartBtn->setFont(m_font);
    QPushButton* pDetectorYStopBtn = new QPushButton(tr("停止"));
    pDetectorYStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorYAixs,10,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorYCurrentPos,10,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorYTargetPos,10,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorYSpeed,10,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorYXStartBtn,10,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorYStopBtn,10,5,Qt::AlignCenter);
    //画z轴
    QLabel* pDetectorZAixs = new QLabel(tr("Z轴 (mm)"));
    pDetectorZAixs->setFont(m_font);
    m_pDetectorZCurrentPos = new GyDoubleSpinBox();
    m_pDetectorZCurrentPos->setFont(m_font);
    m_pDetectorZTargetPos = new GyDoubleSpinBox();
    m_pDetectorZTargetPos->setFont(m_font);
    connect(m_pDetectorZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pDetectorZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorZSpeed = new GyDoubleSpinBox();
    m_pDetectorZSpeed->setFont(m_font);
    connect(m_pDetectorZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&GongYTabWidget::slotAxisPosChanged);
    connect(m_pDetectorZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    QPushButton* pDetectorZXStartBtn = new QPushButton(tr("开始"));
    pDetectorZXStartBtn->setFont(m_font);
    QPushButton* pDetectorZStopBtn = new QPushButton(tr("停止"));
    pDetectorZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorZAixs,11,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZCurrentPos,11,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZTargetPos,11,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZSpeed,11,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXStartBtn,11,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZStopBtn,11,5,Qt::AlignCenter);


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

    double val = pSendBox->text().toDouble();
    qDebug()<<pSendBox->text();
}

void GongYTabWidget::slotAxisSpeedChanged()
{
    QLineEdit* pSendBox = qobject_cast<QLineEdit*>(sender());
    if(pSendBox == nullptr)
        return;

    double val = pSendBox->text().toDouble();
    qDebug()<<pSendBox->text();
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
