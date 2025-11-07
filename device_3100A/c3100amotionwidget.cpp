#include "device_3100A/C3100AMotionWidget.h"
#include "device_3100A/IMotionController.h"
#include "device_3100A/IXrayController.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDebug>
#include <QVariant>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QRadioButton>

struct MotionCtrlInfoPlaceholder {
    double axEncPos;
};

C3100AMotionWidget::C3100AMotionWidget(IMotionController* motionCtl, IXrayController* xrayCtl, QWidget *parent)
    : QTabWidget(parent),
    m_motionController(motionCtl),
    m_xrayController(xrayCtl),
    m_font("Microsoft YaHei", 9)
{
    // 1. 设置Tab风格
    setTabPosition(QTabWidget::North);
    setTabShape(QTabWidget::Rounded);

    // 2. 构建UI
    setupTabs();

    // 3. 连接来自控制器的信号 (功能实现)
    if (m_motionController)
    {
        // connect(m_motionController, &IMotionController::sig_axisDataUpdated,
        //         this, &C3100AMotionWidget::slot_DataChanged);
    }

    // 连接 XRay 控制器信号
    if (m_xrayController)
    {
        connect(m_xrayController, &IXrayController::sig_statusUpdated,
                this, &C3100AMotionWidget::slot_xrayStatusUpdated);
        connect(m_xrayController, &IXrayController::sig_connected,
                this, &C3100AMotionWidget::slot_xrayConnected);
    }
}

C3100AMotionWidget::~C3100AMotionWidget()
{
}

void C3100AMotionWidget::setupTabs()
{
    // 只创建一个Tab
    addTab(createMotionTab(), tr("3100A 运动控制"));
}

QWidget* C3100AMotionWidget::createMotionTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setSpacing(10); // 设置间距

    createXrayGroup(layout);          // 1. 射线机
    createMotionControlGroup(layout); // 2. 运动控制 (我们之前已完成)
    createAcquireGroup(layout);       // 3. 采集图片

    layout->addStretch(); // 占位符，将所有内容推到顶部
    tab->setLayout(layout);
    return tab;
}

/**
 * @brief 创建射线机 GoupBox
 */
void C3100AMotionWidget::createXrayGroup(QVBoxLayout* pGroupLayout)
{
    QGroupBox* pXrayGroup = new QGroupBox(tr("射线机"));
    pXrayGroup->setFont(QFont(m_font.family(), 10, QFont::Bold));
    pXrayGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout* pXrayLayout = new QVBoxLayout(pXrayGroup);

    // 红色标题
    QLabel* pLabel = new QLabel(tr("Hamamatsu"));
    pLabel->setFixedHeight(25);
    pLabel->setAlignment(Qt::AlignCenter);
    pLabel->setFont(m_font);
    pLabel->setStyleSheet("border: none; background-color: rgb(255, 0, 0); color: white;");
    pXrayLayout->addWidget(pLabel);

    // --- 电压和电流 ---
    QHBoxLayout* pVolCurrentHLayout = new QHBoxLayout();

    // 电压
    QGroupBox* pVoltageGroup = new QGroupBox(tr("Voltage(kV)"));
    pVoltageGroup->setFont(m_font);
    pVoltageGroup->setMaximumHeight(100);
    QFormLayout* pvolFormLayout = new QFormLayout();
    m_pVolCurrentLineEdit = new QLineEdit();
    m_pVolCurrentLineEdit->setFont(m_font);
    m_pVolCurrentLineEdit->setEnabled(false);
    m_pVolTargetSpinbox = new C3100AIntSpinBox();
    m_pVolTargetSpinbox->setRange(0, 500);
    m_pVolTargetSpinbox->setFont(m_font);
    connect(m_pVolTargetSpinbox, &C3100AIntSpinBox::editingFinished, this, &C3100AMotionWidget::slotVoltageChanged);
    pvolFormLayout->addRow(tr("Current:"), m_pVolCurrentLineEdit);
    pvolFormLayout->addRow(tr("Target:"), m_pVolTargetSpinbox);
    pVoltageGroup->setLayout(pvolFormLayout);
    pVolCurrentHLayout->addWidget(pVoltageGroup);

    // 电流
    QGroupBox* pCurrentGroup = new QGroupBox(tr("Current(uA)"));
    pCurrentGroup->setFont(m_font);
    pCurrentGroup->setMaximumHeight(100);
    QFormLayout* pcurrentFormLayout = new QFormLayout();
    m_pCurrentCurrentLineEdit = new QLineEdit();
    m_pCurrentCurrentLineEdit->setFont(m_font);
    m_pCurrentCurrentLineEdit->setEnabled(false);
    m_pCurrentTargetSpinbox = new C3100AIntSpinBox();
    m_pCurrentTargetSpinbox->setRange(0, 1000);
    m_pCurrentTargetSpinbox->setFont(m_font);
    connect(m_pCurrentTargetSpinbox, &C3100AIntSpinBox::editingFinished, this, &C3100AMotionWidget::slotCurrentChanged);
    pcurrentFormLayout->addRow(tr("Current"), m_pCurrentCurrentLineEdit);
    pcurrentFormLayout->addRow(tr("Target"), m_pCurrentTargetSpinbox);
    pCurrentGroup->setLayout(pcurrentFormLayout);
    pVolCurrentHLayout->addWidget(pCurrentGroup);

    pXrayLayout->addLayout(pVolCurrentHLayout);

    // --- XON / XOFF 按钮 ---
    QHBoxLayout* pBtnsLayout = new QHBoxLayout();
    pBtnsLayout->addStretch();
    m_pButtonXon = new QPushButton(tr("XON"));
    m_pButtonXon->setFont(m_font);
    connect(m_pButtonXon, &QPushButton::clicked, this, &C3100AMotionWidget::slotXonClick);
    pBtnsLayout->addWidget(m_pButtonXon);
    pBtnsLayout->addStretch();
    m_pButtonXoff = new QPushButton(tr("XOFF"));
    m_pButtonXoff->setFont(m_font);
    connect(m_pButtonXoff, &QPushButton::clicked, this, &C3100AMotionWidget::slotXoffClick);
    pBtnsLayout->addWidget(m_pButtonXoff);
    pBtnsLayout->addStretch();
    pXrayLayout->addLayout(pBtnsLayout);

    // --- 状态 CheckBox ---
    QGroupBox* pChecksGroup = new QGroupBox();
    pChecksGroup->setFont(m_font);
    pChecksGroup->setMaximumHeight(50);
    QHBoxLayout* pStatesLayout = new QHBoxLayout(pChecksGroup);
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

    pXrayLayout->addWidget(pChecksGroup);

    pGroupLayout->addWidget(pXrayGroup);
}

/**
 * @brief 创建采集图片 GoupBox
 */
void C3100AMotionWidget::createAcquireGroup(QVBoxLayout* pGroupLayout)
{
    QGroupBox* pAcquireGroup = new QGroupBox(tr("采集图片"));
    pAcquireGroup->setFont(QFont(m_font.family(), 10, QFont::Bold));
    pAcquireGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    pAcquireGroup->setMaximumHeight(80); // 限制高度

    QVBoxLayout* pAcquireVLayout = new QVBoxLayout(pAcquireGroup);
    QHBoxLayout* pAcquireModeHLayout = new QHBoxLayout();

    QLabel* pModelLabel = new QLabel(tr("采集模式"));
    pModelLabel->setFont(m_font);

    m_pStepRadioBtn = new QRadioButton(tr("步进"));
    m_pStepRadioBtn->setFont(m_font);
    m_pStepRadioBtn->setChecked(true); // 默认选中

    m_pContineRadioBtn = new QRadioButton(tr("连续"));
    m_pContineRadioBtn->setFont(m_font);

    m_pDrRadioBtn = new QRadioButton(tr("DR"));
    m_pDrRadioBtn->setFont(m_font);

    m_pSpiralRadioBtn = new QRadioButton(tr("螺旋"));
    m_pSpiralRadioBtn->setFont(m_font);

    m_pBiasRadioBtn = new QRadioButton(tr("偏置"));
    m_pBiasRadioBtn->setFont(m_font);

    pAcquireModeHLayout->addWidget(pModelLabel);
    pAcquireModeHLayout->addSpacing(10);
    pAcquireModeHLayout->addWidget(m_pStepRadioBtn);
    pAcquireModeHLayout->addWidget(m_pContineRadioBtn);
    pAcquireModeHLayout->addWidget(m_pDrRadioBtn);
    pAcquireModeHLayout->addWidget(m_pSpiralRadioBtn);
    pAcquireModeHLayout->addWidget(m_pBiasRadioBtn);
    pAcquireModeHLayout->addStretch(); // 占满剩余空间

    pAcquireVLayout->addLayout(pAcquireModeHLayout);
    pGroupLayout->addWidget(pAcquireGroup);
}

/**
 * @brief 创建运动控制的主GoupBox和Grid布局
 */
void C3100AMotionWidget::createMotionControlGroup(QVBoxLayout* pGroupLayout)
{
    QGroupBox* pMotionCtrlGroup = new QGroupBox(tr("运动控制"));
    pMotionCtrlGroup->setFont(QFont(m_font.family(), 10, QFont::Bold));
    pMotionCtrlGroup->setStyleSheet("QGroupBox { font-weight: bold; }");

    QGridLayout* pGridLayout = new QGridLayout(pMotionCtrlGroup);
    pGridLayout->setSpacing(10); // 设置间距

    // --- 1. 创建标题栏 ---
    int col = 0;
    pGridLayout->addWidget(new QLabel(tr("轴名称")), 0, col++, Qt::AlignCenter);
    pGridLayout->addWidget(new QLabel(tr("当前位置")), 0, col++, Qt::AlignCenter);
    pGridLayout->addWidget(new QLabel(tr("目标位置")), 0, col++, Qt::AlignCenter);
    pGridLayout->addWidget(new QLabel(tr("速度")), 0, col++, Qt::AlignCenter);
    pGridLayout->addWidget(new QLabel(tr("JOG")), 0, col, 1, 2, Qt::AlignCenter); col+=2;
    pGridLayout->addWidget(new QLabel(tr("操作")), 0, col, 1, 2, Qt::AlignCenter); col+=2;

    QPushButton* pStopAllBtn = new QPushButton(tr("全部停止"));
    pStopAllBtn->setFont(m_font);
    pStopAllBtn->setStyleSheet("background-color: #FF6B6B; color: white;"); // 红色停止按钮
    connect(pStopAllBtn, &QPushButton::clicked, this, &C3100AMotionWidget::slotStopAllBtnClick);
    pGridLayout->addWidget(pStopAllBtn, 0, col++, Qt::AlignCenter);

    // --- 2. 创建轴控制行 ---
    int row = 1;
    createAxisRow(pGridLayout, row++, AXIS_3100A::XRAY_Z,            tr("射线源 Z 轴"));
    createAxisRow(pGridLayout, row++, AXIS_3100A::Detector_R,       tr("探测器 R 轴"));
    createAxisRow(pGridLayout, row++, AXIS_3100A::Detector_W,        tr("探测器Theta轴"));
    createAxisRow(pGridLayout, row++, AXIS_3100A::objectiveTable_X1, tr("载物台 X 轴"));
    createAxisRow(pGridLayout, row++, AXIS_3100A::objectiveTable_Y1, tr("载物台 Y 轴"));
    createAxisRow(pGridLayout, row++, AXIS_3100A::Detector_Z1,       tr("探测器 Z1 轴"));

    pGroupLayout->addWidget(pMotionCtrlGroup);
}

/**
 * @brief 辅助函数：创建并添加一整行的轴控件
 */
void C3100AMotionWidget::createAxisRow(QGridLayout* layout, int row, int axisId, const QString& name)
{
    // 1. 轴名称
    QLabel* pLabel = new QLabel(name);
    pLabel->setFont(m_font);
    pLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 2. 当前位置 (只读)
    C3100ADoubleSpinBox* pCurrentPos = new C3100ADoubleSpinBox(axisId);
    pCurrentPos->setEnabled(false);
    pCurrentPos->setStyleSheet("background-color: #f0f0f0;"); // 灰色背景
    m_currentPosWidgets[axisId] = pCurrentPos; // 存入Map

    // 3. 目标位置 (可写)
    C3100ADoubleSpinBox* pTargetPos = new C3100ADoubleSpinBox(axisId);
    connect(pTargetPos, &C3100ADoubleSpinBox::editingFinished, this, &C3100AMotionWidget::slotAxisPosChanged);
    m_targetPosWidgets[axisId] = pTargetPos; // 存入Map

    // 4. 速度 (可写)
    C3100ADoubleSpinBox* pSpeed = new C3100ADoubleSpinBox(axisId);
    pSpeed->setDecimals(2); // 速度用2位小数
    pSpeed->setRange(0.1, 500.0); // 假设的速度范围
    pSpeed->setValue(50.0);       // 默认速度
    connect(pSpeed, &C3100ADoubleSpinBox::editingFinished, this, &C3100AMotionWidget::slotAxisSpeedChanged);
    m_speedWidgets[axisId] = pSpeed; // 存入Map

    // 5. JOG 按钮
    C3100APushButton* pLeftBtn = new C3100APushButton(axisId, tr("<<"));
    connect(pLeftBtn, &QPushButton::pressed, this, &C3100AMotionWidget::slotLeftButtonPress);
    connect(pLeftBtn, &QPushButton::released, this, &C3100AMotionWidget::slotButtonRelease);

    C3100APushButton* pRightBtn = new C3100APushButton(axisId, tr(">>"));
    connect(pRightBtn, &QPushButton::pressed, this, &C3100AMotionWidget::slotRightButtonPress);
    connect(pRightBtn, &QPushButton::released, this, &C3100AMotionWidget::slotButtonRelease);

    // 6. 操作按钮
    C3100APushButton* pHomeBtn = new C3100APushButton(axisId, tr("回零"));
    connect(pHomeBtn, &QPushButton::clicked, this, &C3100AMotionWidget::slotHomeButtonClick);

    C3100APushButton* pStopBtn = new C3100APushButton(axisId, tr("停止"));
    pStopBtn->setStyleSheet("background-color: #FFB36B;"); // 橙色停止按钮
    connect(pStopBtn, &QPushButton::clicked, this, &C3100AMotionWidget::slotStopButtonClick);

    // --- 添加到网格 ---
    int col = 0;
    layout->addWidget(pLabel, row, col++, Qt::AlignRight);
    layout->addWidget(pCurrentPos, row, col++, Qt::AlignCenter);
    layout->addWidget(pTargetPos, row, col++, Qt::AlignCenter);
    layout->addWidget(pSpeed, row, col++, Qt::AlignCenter);
    layout->addWidget(pLeftBtn, row, col++, Qt::AlignCenter);
    layout->addWidget(pRightBtn, row, col++, Qt::AlignCenter);
    layout->addWidget(pHomeBtn, row, col++, Qt::AlignCenter);
    layout->addWidget(pStopBtn, row, col++, Qt::AlignCenter);
    // 第8列是“全部停止”按钮，在标题行
}


void C3100AMotionWidget::slot_DataChanged(const QVariant &var)
{
    // TODO: 这一步需要您提供 `MotionCtrlData::MotionCtrlInfo` 的定义
    // (通常是包含 motionctrlcommand.h)

    // 伪代码，演示如何工作:
    /*
    QVector<MotionCtrlInfoPlaceholder> mcVec = var.value<QVector<MotionCtrlInfoPlaceholder>>();

    for(int axisId : m_currentPosWidgets.keys())
    {
        if (axisId < mcVec.size()) {
            // 用从控制器获取的真实位置更新UI
            m_currentPosWidgets[axisId]->setValue(mcVec[axisId].axEncPos);
        }
    }
    */
    qDebug() << "[UI] slot_DataChanged: Received data (update logic pending)." << var;
}

void C3100AMotionWidget::slotAxisPosChanged()
{
    // 1. 获取是哪个SpinBox触发的
    auto* spinBox = qobject_cast<C3100ADoubleSpinBox*>(sender());
    if (!spinBox) return;

    int axisId = spinBox->axisId();
    double pos = spinBox->value();

    qDebug() << "[UI] slotAxisPosChanged: Axis" << axisId << "target set to" << pos;

    // 2. (功能) 调用控制器接口
    if (m_motionController) {
        // m_motionController->ptpMove(axisId, pos);
    }
}

void C3100AMotionWidget::slotAxisSpeedChanged()
{
    auto* spinBox = qobject_cast<C3100ADoubleSpinBox*>(sender());
    if (!spinBox) return;

    int axisId = spinBox->axisId();
    double speed = spinBox->value();

    qDebug() << "[UI] slotAxisSpeedChanged: Axis" << axisId << "speed set to" << speed;

    // 3. (功能)
}

void C3100AMotionWidget::slotLeftButtonPress()
{
    auto* btn = qobject_cast<C3100APushButton*>(sender());
    if (!btn) return;

    int axisId = btn->axisId();
    qDebug() << "[UI] slotLeftButtonPress: Axis" << axisId << "JOG negative";

    // (功能)
    if (m_motionController) {
        // double speed = m_speedWidgets[axisId]->value(); // 获取速度
        // m_motionController->jogMove(axisId, false); // false = 反向
    }
}

void C3100AMotionWidget::slotRightButtonPress()
{
    auto* btn = qobject_cast<C3100APushButton*>(sender());
    if (!btn) return;

    int axisId = btn->axisId();
    qDebug() << "[UI] slotRightButtonPress: Axis" << axisId << "JOG positive";

    // (功能)
    if (m_motionController) {
        // double speed = m_speedWidgets[axisId]->value(); // 获取速度
        // m_motionController->jogMove(axisId, true); // true = 正向
    }
}

void C3100AMotionWidget::slotButtonRelease()
{
    auto* btn = qobject_cast<C3100APushButton*>(sender());
    if (!btn) return;

    int axisId = btn->axisId();
    qDebug() << "[UI] slotButtonRelease: Axis" << axisId << "JOG stop";

    // (功能) JOG 释放 = 停止
    if (m_motionController) {
        // m_motionController->stopAxis(axisId);
    }
}

void C3100AMotionWidget::slotHomeButtonClick()
{
    auto* btn = qobject_cast<C3100APushButton*>(sender());
    if (!btn) return;

    int axisId = btn->axisId();
    qDebug() << "[UI] slotHomeButtonClick: Axis" << axisId << "HOMING";

    // (功能)
    if (m_motionController) {
        // m_motionController->homeAxis(axisId);
    }
}

void C3100AMotionWidget::slotStopButtonClick()
{
    auto* btn = qobject_cast<C3100APushButton*>(sender());
    if (!btn) return;

    int axisId = btn->axisId();
    qDebug() << "[UI] slotStopButtonClick: Axis" << axisId << "STOP";

    // (功能)
    if (m_motionController) {
        // m_motionController->stopAxis(axisId);
    }
}

void C3100AMotionWidget::slotStopAllBtnClick()
{
    qDebug() << "[UI] slotStopAllBtnClick: STOP ALL AXES";

    // (功能)
    if (m_motionController) {
        // m_motionController->stopAll();
    }
}

void C3100AMotionWidget::slotXonClick()
{
    qDebug() << "[UI] XON Clicked";
    // (功能)
    if (m_xrayController) {
        m_xrayController->setXrayOn();
    }
}

void C3100AMotionWidget::slotXoffClick()
{
    qDebug() << "[UI] XOFF Clicked";
    // (功能)
    if (m_xrayController) {
        m_xrayController->setXrayOff();
    }
}

void C3100AMotionWidget::slotVoltageChanged()
{
    auto* spinBox = qobject_cast<C3100AIntSpinBox*>(sender());
    if (!spinBox) return;

    int kv = spinBox->value();
    qDebug() << "[UI] Voltage Target set to:" << kv;

    // (功能)
    if (m_xrayController) {
        m_xrayController->setVoltage(kv);
    }
}

void C3100AMotionWidget::slotCurrentChanged()
{
    auto* spinBox = qobject_cast<C3100AIntSpinBox*>(sender());
    if (!spinBox) return;

    int ua = spinBox->value();
    qDebug() << "[UI] Current Target set to:" << ua;

    if (m_xrayController) {
        m_xrayController->setCurrent(ua);
    }
}

void C3100AMotionWidget::slot_xrayStatusUpdated(bool warmup, bool locked, bool on, int kv, int ua)
{
    // 更新UI
    m_pVolCurrentLineEdit->setText(QString::number(kv));
    m_pCurrentCurrentLineEdit->setText(QString::number(ua));

    m_pWarmUp->setChecked(warmup);
    m_pInterLock->setChecked(locked);
    m_pXray->setChecked(on);

    // 根据互锁和预热状态，控制XON/XOFF按钮的可用性
    // 只有在 (已连接) 且 (未互锁) 且 (未预热) 时才允许操作
    bool canOperate = m_pConnect->isChecked() && !locked && !warmup;
    m_pButtonXon->setEnabled(canOperate && !on);
    m_pButtonXoff->setEnabled(canOperate && on);
}

void C3100AMotionWidget::slot_xrayConnected(bool connected)
{
    m_pConnect->setChecked(connected);

    // 连接断开时，禁用所有操作按钮
    if (!connected)
    {
        m_pButtonXon->setEnabled(false);
        m_pButtonXoff->setEnabled(false);
    }
    else
    {
        // 刚连接上时，按钮状态取决于第一次状态更新
        // (也可以在这里主动请求一次状态，但C3100AXrayController的定时器会处理)
    }
}
