#include "gongy.h"
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
    if(m_pTimer->isActive())
    {
        m_pTimer->stop();
        m_slash=false;
    }
    else
    {
        m_pTimer->start(50);
    }
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
    pXrayGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");

    QVBoxLayout* pXrayLayout = new QVBoxLayout(pXrayGroup);

    m_pLabel = new QLabel(tr("Hamamatsu"));
    m_pLabel->setFixedHeight(30);
    m_pLabel->setAlignment(Qt::AlignCenter);
    m_pLabel->setFont(m_font);
    m_pLabel->setStyleSheet("border: none; background-color: rgb(255, 0, 0);");
    pXrayLayout->addWidget(m_pLabel);

    QHBoxLayout* pVolCurrentHLayout = new QHBoxLayout();
    QGroupBox* pVoltageGroup = new QGroupBox(tr("Voltage(kV)"));
    pVoltageGroup->setFont(m_font);
    pVoltageGroup->setMaximumHeight(100);
    pVoltageGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout* pvolFormLayout = new QFormLayout();
    m_pVolCurrentLineEdit = new QLineEdit();
    m_pVolCurrentLineEdit->setFont(m_font);
    m_pVolTargetLineEdit = new QLineEdit();
    m_pVolTargetLineEdit->setFont(m_font);
    connect(m_pVolTargetLineEdit,&QLineEdit::returnPressed,this,&GongYTabWidget::slotVoltageChanged);
    connect(m_pVolTargetLineEdit,&QLineEdit::editingFinished,this,&GongYTabWidget::slotVoltageChanged);
    pvolFormLayout->addRow(tr("Current:"),m_pVolCurrentLineEdit);
    pvolFormLayout->addRow(tr("Target:"),m_pVolTargetLineEdit);
    pVoltageGroup->setLayout(pvolFormLayout);
    pVolCurrentHLayout->addWidget(pVoltageGroup);

    QGroupBox* pCurrentGroup = new QGroupBox(tr("Current(uA)"));
    pCurrentGroup->setFont(m_font);
    pCurrentGroup->setMaximumHeight(100);
    pCurrentGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout* pcurrentFormLayout = new QFormLayout();
    m_pCurrentCurrentLineEdit = new QLineEdit();
    m_pCurrentCurrentLineEdit->setFont(m_font);
    m_pCurrentTargetLineEdit = new QLineEdit();
    m_pCurrentTargetLineEdit->setFont(m_font);
    connect(m_pCurrentTargetLineEdit,&QLineEdit::returnPressed,this,&GongYTabWidget::slotCurrentChanged);
    connect(m_pCurrentTargetLineEdit,&QLineEdit::editingFinished,this,&GongYTabWidget::slotCurrentChanged);
    pcurrentFormLayout->addRow(tr("Current"),m_pCurrentCurrentLineEdit);
    pcurrentFormLayout->addRow(tr("Target"),m_pCurrentTargetLineEdit);
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
    m_pWarmUp = new QCheckBox(tr("Warm Up"));
    m_pInterLock = new QCheckBox(tr("Inter lock"));
    m_pConnect = new QCheckBox(tr("Connect"));
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

    //ox轴
    QPushButton* pHomeBtn = new QPushButton(tr("归零"));
    pHomeBtn->setFont(m_font);
    QLabel* pCalibrationLable = new QLabel(tr("标定编号:"));
    pCalibrationLable->setAlignment(Qt::AlignCenter);
    pCalibrationLable->setFont(m_font);
    QComboBox* pAxleCombox = new QComboBox();
    pAxleCombox->setFont(m_font);
    QPushButton* pEditBtn = new QPushButton(tr("编辑"));
    pEditBtn->setFont(m_font);
    pGridLayout->addWidget(pHomeBtn,0,1);
    pGridLayout->addWidget(pCalibrationLable,0,2);
    pGridLayout->addWidget(pAxleCombox,0,3);
    pGridLayout->addWidget(pEditBtn,0,4);

    QLabel* pAxelLabel = new QLabel(tr("OX轴:"));
    pAxelLabel->setAlignment(Qt::AlignVCenter);
    pAxelLabel->setFont(m_font);
    QLabel* pCurrentPosLabel = new QLabel(tr("当前位置:"));
    pCurrentPosLabel->setAlignment(Qt::AlignCenter);
    pCurrentPosLabel->setFont(m_font);
    QLineEdit* pCurrentPosEdit = new QLineEdit();
    pCurrentPosEdit->setFont(m_font);
    QLabel* pCurrentVLabel = new QLabel(tr("当前速度:"));
    pCurrentVLabel->setAlignment(Qt::AlignCenter);
    pCurrentVLabel->setFont(m_font);
    QLineEdit* pCurrentVEdit = new QLineEdit();
    pCurrentVEdit->setFont(m_font);
    pGridLayout->addWidget(pAxelLabel,1,0);
    pGridLayout->addWidget(pCurrentPosLabel,1,1);
    pGridLayout->addWidget(pCurrentPosEdit,1,2);
    pGridLayout->addWidget(pCurrentVLabel,1,3);
    pGridLayout->addWidget(pCurrentVEdit,1,4);

    QLabel* pLocPosLabel = new QLabel(tr("定位位置:"));
    pLocPosLabel->setAlignment(Qt::AlignCenter);
    pLocPosLabel->setFont(m_font);
    QLineEdit* pLocPosEdit = new QLineEdit();
    pLocPosEdit->setFont(m_font);
    QLabel* pMoveVLabel = new QLabel(tr("移动速度:"));
    pMoveVLabel->setAlignment(Qt::AlignCenter);
    pMoveVLabel->setFont(m_font);
    QLineEdit* pMoveVEdit = new QLineEdit();
    pMoveVEdit->setFont(m_font);
    pGridLayout->addWidget(pLocPosLabel,2,1);
    pGridLayout->addWidget(pLocPosEdit,2,2);
    pGridLayout->addWidget(pMoveVLabel,2,3);
    pGridLayout->addWidget(pMoveVEdit,2,4);

    QPushButton* pPBtn = new QPushButton(tr("正"));
    pPBtn->setFont(m_font);
    pPBtn->setFixedWidth(60);
    QPushButton* pNBtn = new QPushButton(tr("反"));
    pNBtn->setFont(m_font);
    pNBtn->setFixedWidth(60);
    QPushButton* pMBtn = new QPushButton(tr("动"));
    pMBtn->setFixedWidth(60);
    pMBtn->setFont(m_font);
    QPushButton* pSBtn = new QPushButton(tr("停"));
    pSBtn->setFixedWidth(60);
    pSBtn->setFont(m_font);
    QHBoxLayout* pBtnsLayout = new QHBoxLayout();
    pBtnsLayout->addWidget(pPBtn);
    pBtnsLayout->addWidget(pNBtn);
    pBtnsLayout->addWidget(pMBtn);
    pBtnsLayout->addWidget(pSBtn);
    pGridLayout->addLayout(pBtnsLayout,3,1,1,5);

    //W轴
    QLabel* pWAxelLabel = new QLabel(tr("W轴:"));
    pWAxelLabel->setAlignment(Qt::AlignVCenter);
    pWAxelLabel->setFont(m_font);
    QLabel* pWCurrentPosLabel = new QLabel(tr("当前位置:"));
    pWCurrentPosLabel->setAlignment(Qt::AlignCenter);
    pWCurrentPosLabel->setFont(m_font);
    QLineEdit* pWCurrentPosEdit = new QLineEdit();
    pWCurrentPosEdit->setFont(m_font);
    QLabel* pWCurrentVLabel = new QLabel(tr("当前速度:"));
    pWCurrentVLabel->setAlignment(Qt::AlignCenter);
    pWCurrentVLabel->setFont(m_font);
    QLineEdit* pWCurrentVEdit = new QLineEdit();
    pWCurrentVEdit->setFont(m_font);
    pGridLayout->addWidget(pWAxelLabel,4,0);
    pGridLayout->addWidget(pWCurrentPosLabel,4,1);
    pGridLayout->addWidget(pWCurrentPosEdit,4,2);
    pGridLayout->addWidget(pWCurrentVLabel,4,3);
    pGridLayout->addWidget(pWCurrentVEdit,4,4);

    QLabel* pWLocPosLabel = new QLabel(tr("定位位置:"));
    pWLocPosLabel->setAlignment(Qt::AlignCenter);
    pWLocPosLabel->setFont(m_font);
    QLineEdit* pWLocPosEdit = new QLineEdit();
    pWLocPosEdit->setFont(m_font);
    QLabel* pWMoveVLabel = new QLabel(tr("移动速度:"));
    pWMoveVLabel->setAlignment(Qt::AlignCenter);
    pWMoveVLabel->setFont(m_font);
    QLineEdit* pWMoveVEdit = new QLineEdit();
    pWMoveVEdit->setFont(m_font);
    pGridLayout->addWidget(pWLocPosLabel,5,1);
    pGridLayout->addWidget(pWLocPosEdit,5,2);
    pGridLayout->addWidget(pWMoveVLabel,5,3);
    pGridLayout->addWidget(pWMoveVEdit,5,4);

    QPushButton* pWPBtn = new QPushButton(tr("正"));
    pWPBtn->setFont(m_font);
    pWPBtn->setFixedWidth(60);
    QPushButton* pWNBtn = new QPushButton(tr("反"));
    pWNBtn->setFont(m_font);
    pWNBtn->setFixedWidth(60);
    QPushButton* pWMBtn = new QPushButton(tr("动"));
    pWMBtn->setFixedWidth(60);
    pWMBtn->setFont(m_font);
    QPushButton* pWSBtn = new QPushButton(tr("停"));
    pWSBtn->setFixedWidth(60);
    pWSBtn->setFont(m_font);
    QHBoxLayout* pWBtnsLayout = new QHBoxLayout();
    pWBtnsLayout->addWidget(pWPBtn);
    pWBtnsLayout->addWidget(pWNBtn);
    pWBtnsLayout->addWidget(pWMBtn);
    pWBtnsLayout->addWidget(pWSBtn);
    pGridLayout->addLayout(pWBtnsLayout,6,1,1,5);


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
        int val = m_pVolTargetLineEdit->text().trimmed().toInt();
        m_pXraySource->setXrayKv(val);
    }
}

void GongYTabWidget::slotCurrentChanged()
{
    if(m_online && m_pXraySource)
    {
        int val = m_pCurrentTargetLineEdit->text().trimmed().toInt();
        m_pXraySource->setXrayBeam(val);
    }
}

void GongYTabWidget::slotGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua)
{
    m_pVolCurrentLineEdit->setText(QString::number(kv));
    m_pCurrentCurrentLineEdit->setText(QString::number(ua));

    m_online=true;

    if (locked) {
        m_pInterLock->setChecked(true);
        m_pWarmUp->setChecked(false);
        m_pXray->setChecked(false);
        m_pConnect->setChecked(false);
    } else if (warmup) {
        m_pWarmUp->setChecked(true);
        m_pInterLock->setChecked(false);
        m_pXray->setChecked(false);
        m_pConnect->setChecked(false);
    } else if (on) {
        m_pXray->setChecked(true);
        m_pInterLock->setChecked(false);
        m_pWarmUp->setChecked(false);
        m_pConnect->setChecked(false);
    } else {
        m_pXray->setChecked(false);
        m_pInterLock->setChecked(false);
        m_pWarmUp->setChecked(false);
        m_pConnect->setChecked(true);
    }
}

