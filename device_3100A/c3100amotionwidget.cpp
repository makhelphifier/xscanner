#include "c3100amotionwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>  // 如果需要调试，可保留

C3100AMotionWidget::C3100AMotionWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setupTabs();
    // 设置默认 Tab
    setCurrentIndex(0);
    // 其他初始化（如 opacity）可在后续添加
}

C3100AMotionWidget::~C3100AMotionWidget()
{
    // 无需删除 UI 控件，Qt 父子关系自动管理
}

float C3100AMotionWidget::opacity() const
{
    // 简化实现，返回默认值；后续可添加成员
    return 1.0f;
}

void C3100AMotionWidget::setOpacity(float value)
{
    // 简化实现；后续可更新 UI 透明度
    // m_opacity = value;
    // update();  // 如果需要重绘
}

void C3100AMotionWidget::setSlashMark(bool slashMark)
{
    // 简化实现（原依赖 m_slash 和 m_pTimer，已移除）；后续可添加
    Q_UNUSED(slashMark);
    // if (slashMark) { ... }
}

void C3100AMotionWidget::setupTabs()
{
    // 设置 Tab 位置和形状
    setTabPosition(QTabWidget::North);
    setTabShape(QTabWidget::Rounded);

    // 添加页签
    addTab(createFirstTab(), tr("扫描设备"));
    addTab(createSecondTab(), tr("图像查看"));
}

QWidget* C3100AMotionWidget::createFirstTab()
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tab);

    createXrayGroup(layout);
    createDetectorGroup(layout);
    // createAcquireGroup(layout);  // 可后续添加，如果需要采集组

    tab->setLayout(layout);
    return tab;
}

QWidget* C3100AMotionWidget::createSecondTab()
{
    QWidget* tab = new QWidget(this);
    // 简化：图像查看 Tab，可后续添加 DetectorWidget 等
    // 示例：QLabel* label = new QLabel(tr("图像查看区域"), tab);
    // QVBoxLayout* layout = new QVBoxLayout(tab);
    // layout->addWidget(label);
    return tab;
}

void C3100AMotionWidget::createXrayGroup(QVBoxLayout* layout)
{
    // 射线机 GroupBox
    QGroupBox* pXrayGroup = new QGroupBox(tr("射线机"));
    pXrayGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout* pXrayLayout = new QVBoxLayout(pXrayGroup);

    // 品牌标签（原 m_pLabel）
    QLabel* pLabel = new QLabel(tr("Hamamatsu"));
    pLabel->setFixedHeight(25);
    pLabel->setAlignment(Qt::AlignCenter);
    pLabel->setStyleSheet("border: none; background-color: rgb(255, 0, 0);");
    pXrayLayout->addWidget(pLabel);

    // 电压和电流 HBox
    QHBoxLayout* pVolCurrentHLayout = new QHBoxLayout();

    // 电压 GroupBox
    QGroupBox* pVoltageGroup = new QGroupBox(tr("Voltage(kV)"));
    pVoltageGroup->setMaximumHeight(100);
    pVoltageGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout* pVolFormLayout = new QFormLayout();
    m_pVolCurrentLineEdit = new QLineEdit();
    m_pVolCurrentLineEdit->setEnabled(false);
    m_pVolTargetSpinbox = new C3100AIntSpinBox();
    m_pVolTargetSpinbox->setRange(0, 500);
    // connect(m_pVolTargetSpinbox->LineEdit(), &QLineEdit::editingFinished, this, &C3100AMotionWidget::slotVoltageChanged);  // 移除槽
    pVolFormLayout->addRow(tr("Current:"), m_pVolCurrentLineEdit);
    pVolFormLayout->addRow(tr("Target:"), m_pVolTargetSpinbox);
    pVoltageGroup->setLayout(pVolFormLayout);
    pVolCurrentHLayout->addWidget(pVoltageGroup);

    // 电流 GroupBox
    QGroupBox* pCurrentGroup = new QGroupBox(tr("Current(uA)"));
    pCurrentGroup->setMaximumHeight(100);
    pCurrentGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout* pCurrentFormLayout = new QFormLayout();
    m_pCurrentCurrentLineEdit = new QLineEdit();
    m_pCurrentCurrentLineEdit->setEnabled(false);
    m_pCurrentTargetSpinbox = new C3100AIntSpinBox();
    m_pCurrentTargetSpinbox->setRange(0, 1000);
    // connect(m_pCurrentTargetSpinbox->LineEdit(), &QLineEdit::editingFinished, this, &C3100AMotionWidget::slotCurrentChanged);  // 移除槽
    pCurrentFormLayout->addRow(tr("Current"), m_pCurrentCurrentLineEdit);
    pCurrentFormLayout->addRow(tr("Target"), m_pCurrentTargetSpinbox);
    pCurrentGroup->setLayout(pCurrentFormLayout);
    pVolCurrentHLayout->addWidget(pCurrentGroup);

    pXrayLayout->addLayout(pVolCurrentHLayout);

    // XON/XOFF 按钮
    QHBoxLayout* pBtnsLayout = new QHBoxLayout();
    pBtnsLayout->addStretch();
    m_pButtonXon = new QPushButton(tr("XON"));
    // connect(m_pButtonXon, &QPushButton::clicked, this, &C3100AMotionWidget::slotXonClick);  // 移除槽
    pBtnsLayout->addWidget(m_pButtonXon);
    pBtnsLayout->addStretch();
    m_pButtonXoff = new QPushButton(tr("XOFF"));
    // connect(m_pButtonXoff, &QPushButton::clicked, this, &C3100AMotionWidget::slotXoffClick);  // 移除槽
    pBtnsLayout->addWidget(m_pButtonXoff);
    pBtnsLayout->addStretch();
    pXrayLayout->addLayout(pBtnsLayout);

    // 状态 CheckBox
    QGroupBox* pChecksGroup = new QGroupBox();
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

void C3100AMotionWidget::createDetectorGroup(QVBoxLayout* layout)
{
    // 运动控制 GroupBox
    QGroupBox* pMotionCtrlGroup = new QGroupBox(tr("运动控制"));
    pMotionCtrlGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QGridLayout* pGridLayout = new QGridLayout(pMotionCtrlGroup);

    // 标题行：当前值 | 目标值 | 速度 | 左 | 右 | 回零 | 停止 | Stop All
    QLabel* pCurrentLabel = new QLabel(tr("当前值"));
    pCurrentLabel->setAlignment(Qt::AlignCenter);
    QLabel* pTargetLabel = new QLabel(tr("目标值"));
    pTargetLabel->setAlignment(Qt::AlignCenter);
    QLabel* pSpeedLabel = new QLabel(tr("速度"));
    pSpeedLabel->setAlignment(Qt::AlignCenter);
    QPushButton* pStopAllBtn = new QPushButton(tr("Stop all"));
    // connect(pStopAllBtn, &QPushButton::clicked, this, &C3100AMotionWidget::slotStopAllBtnClick);  // 移除槽
    pGridLayout->addWidget(pCurrentLabel, 0, 1, Qt::AlignCenter);
    pGridLayout->addWidget(pTargetLabel, 0, 2, Qt::AlignCenter);
    pGridLayout->addWidget(pSpeedLabel, 0, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pStopAllBtn, 0, 7, 1, 1, Qt::AlignCenter);  // Stop All 在最后一列

    // 射线源 Z 轴 (AXIS::XRAY_Z) - 行 1
    QLabel* pXrayZLabel = new QLabel(tr("射线源 Z轴 (mm)"));
    pXrayZLabel->setAlignment(Qt::AlignCenter);
    m_pXrayZCurrentPos = new C3100ADoubleSpinBox();
    m_pXrayZCurrentPos->setEnabled(false);  // 当前值通常只读
    m_pXrayZTargetPos = new C3100ADoubleSpinBox();
    m_pXrayZTargetPos->setRange(-100, 1000);
    // connect(m_pXrayZTargetPos->LineEdit(), &QLineEdit::returnPressed, this, &C3100AMotionWidget::slotAxisPosChanged);  // 移除
    m_pXrayZSpeed = new C3100ADoubleSpinBox();
    m_pXrayZSpeed->setRange(0, 100);  // 示例速度范围
    // connect(m_pXrayZSpeed->LineEdit(), &QLineEdit::returnPressed, this, &C3100AMotionWidget::slotAxisSpeedChanged);  // 移除
    QPushButton* pXrayZLeftBtn = new QPushButton(tr("向左"));
    // connect(pXrayZLeftBtn, &QPushButton::pressed, this, &C3100AMotionWidget::slotLeftButtonPress);  // 移除
    QPushButton* pXrayZRightBtn = new QPushButton(tr("向右"));
    // connect(pXrayZRightBtn, &QPushButton::pressed, this, &C3100AMotionWidget::slotRightButtonPress);  // 移除
    QPushButton* pXrayZHomeBtn = new QPushButton(tr("回零"));
    // connect(pXrayZHomeBtn, &QPushButton::clicked, this, &C3100AMotionWidget::slotHomeButtonClick);  // 移除
    QPushButton* pXrayZStopBtn = new QPushButton(tr("停止"));
    // connect(pXrayZStopBtn, &QPushButton::clicked, this, &C3100AMotionWidget::slotEndButtonClick);  // 移除
    pGridLayout->addWidget(pXrayZLabel, 1, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZCurrentPos, 1, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZTargetPos, 1, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pXrayZSpeed, 1, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZLeftBtn, 1, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZRightBtn, 1, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZHomeBtn, 1, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pXrayZStopBtn, 1, 7, Qt::AlignCenter);

    // 探测器 Z1 轴 (AXIS::Detector_Z1) - 行 2
    QLabel* pDetectorZ1Label = new QLabel(tr("探测器 Z1轴 (mm)"));
    pDetectorZ1Label->setAlignment(Qt::AlignCenter);
    m_pDetectorZ1CurrentPos = new C3100ADoubleSpinBox();
    m_pDetectorZ1CurrentPos->setEnabled(false);
    m_pDetectorZ1TargetPos = new C3100ADoubleSpinBox();
    m_pDetectorZ1TargetPos->setRange(-100, 1000);
    m_pDetectorZ1Speed = new C3100ADoubleSpinBox();
    m_pDetectorZ1Speed->setRange(0, 100);
    QPushButton* pDetectorZ1LeftBtn = new QPushButton(tr("向左"));
    QPushButton* pDetectorZ1RightBtn = new QPushButton(tr("向右"));
    QPushButton* pDetectorZ1HomeBtn = new QPushButton(tr("回零"));
    QPushButton* pDetectorZ1StopBtn = new QPushButton(tr("停止"));
    pGridLayout->addWidget(pDetectorZ1Label, 2, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZ1CurrentPos, 2, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZ1TargetPos, 2, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZ1Speed, 2, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZ1LeftBtn, 2, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZ1RightBtn, 2, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZ1HomeBtn, 2, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZ1StopBtn, 2, 7, Qt::AlignCenter);

    // 探测器 R 轴 (AXIS::Detector_R) - 行 3
    QLabel* pDetectorRLabel = new QLabel(tr("探测器 R轴 (mm)"));
    pDetectorRLabel->setAlignment(Qt::AlignCenter);
    m_pDetectorRCurrentPos = new C3100ADoubleSpinBox();
    m_pDetectorRCurrentPos->setEnabled(false);
    m_pDetectorRTargetPos = new C3100ADoubleSpinBox();
    m_pDetectorRTargetPos->setRange(-100, 1000);
    m_pDetectorRSpeed = new C3100ADoubleSpinBox();
    m_pDetectorRSpeed->setRange(0, 100);
    QPushButton* pDetectorRLeftBtn = new QPushButton(tr("向左"));
    QPushButton* pDetectorRRightBtn = new QPushButton(tr("向右"));
    QPushButton* pDetectorRHomeBtn = new QPushButton(tr("回零"));
    QPushButton* pDetectorRStopBtn = new QPushButton(tr("停止"));
    pGridLayout->addWidget(pDetectorRLabel, 3, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorRCurrentPos, 3, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorRTargetPos, 3, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorRSpeed, 3, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRLeftBtn, 3, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRRightBtn, 3, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRHomeBtn, 3, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRStopBtn, 3, 7, Qt::AlignCenter);

    // 探测器 theta 轴 (AXIS::Detector_Theta) - 行 4
    QLabel* pDetectorThetaLabel = new QLabel(tr("探测器 theta轴 (°))"));
    pDetectorThetaLabel->setAlignment(Qt::AlignCenter);
    m_pDetectorThetaCurrentPos = new C3100ADoubleSpinBox();
    m_pDetectorThetaCurrentPos->setEnabled(false);
    m_pDetectorThetaTargetPos = new C3100ADoubleSpinBox();
    m_pDetectorThetaTargetPos->setRange(-360, 360);  // 角度轴范围调整
    m_pDetectorThetaSpeed = new C3100ADoubleSpinBox();
    m_pDetectorThetaSpeed->setRange(0, 100);
    QPushButton* pDetectorThetaLeftBtn = new QPushButton(tr("向左"));
    QPushButton* pDetectorThetaRightBtn = new QPushButton(tr("向右"));
    QPushButton* pDetectorThetaHomeBtn = new QPushButton(tr("回零"));
    QPushButton* pDetectorThetaStopBtn = new QPushButton(tr("停止"));
    pGridLayout->addWidget(pDetectorThetaLabel, 4, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorThetaCurrentPos, 4, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorThetaTargetPos, 4, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorThetaSpeed, 4, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaLeftBtn, 4, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaRightBtn, 4, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaHomeBtn, 4, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaStopBtn, 4, 7, Qt::AlignCenter);

    // 载物台 X 轴 (AXIS::ObjectiveTable_X) - 行 5
    QLabel* pObjectiveTableXLabel = new QLabel(tr("载物台 X轴 (mm)"));
    pObjectiveTableXLabel->setAlignment(Qt::AlignCenter);
    m_pObjectiveTableXCurrentPos = new C3100ADoubleSpinBox();
    m_pObjectiveTableXCurrentPos->setEnabled(false);
    m_pObjectiveTableXTargetPos = new C3100ADoubleSpinBox();
    m_pObjectiveTableXTargetPos->setRange(-100, 1000);
    m_pObjectiveTableXSpeed = new C3100ADoubleSpinBox();
    m_pObjectiveTableXSpeed->setRange(0, 100);
    QPushButton* pObjectiveTableXLeftBtn = new QPushButton(tr("向左"));
    QPushButton* pObjectiveTableXRightBtn = new QPushButton(tr("向右"));
    QPushButton* pObjectiveTableXHomeBtn = new QPushButton(tr("回零"));
    QPushButton* pObjectiveTableXStopBtn = new QPushButton(tr("停止"));
    pGridLayout->addWidget(pObjectiveTableXLabel, 5, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableXCurrentPos, 5, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableXTargetPos, 5, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableXSpeed, 5, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXLeftBtn, 5, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXRightBtn, 5, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXHomeBtn, 5, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXStopBtn, 5, 7, Qt::AlignCenter);

    // 载物台 Y 轴 (AXIS::ObjectiveTable_Y) - 行 6
    QLabel* pObjectiveTableYLabel = new QLabel(tr("载物台 Y轴 (mm)"));
    pObjectiveTableYLabel->setAlignment(Qt::AlignCenter);
    m_pObjectiveTableYCurrentPos = new C3100ADoubleSpinBox();
    m_pObjectiveTableYCurrentPos->setEnabled(false);
    m_pObjectiveTableYTargetPos = new C3100ADoubleSpinBox();
    m_pObjectiveTableYTargetPos->setRange(-100, 1000);
    m_pObjectiveTableYSpeed = new C3100ADoubleSpinBox();
    m_pObjectiveTableYSpeed->setRange(0, 100);
    QPushButton* pObjectiveTableYLeftBtn = new QPushButton(tr("向左"));
    QPushButton* pObjectiveTableYRightBtn = new QPushButton(tr("向右"));
    QPushButton* pObjectiveTableYHomeBtn = new QPushButton(tr("回零"));
    QPushButton* pObjectiveTableYStopBtn = new QPushButton(tr("停止"));
    pGridLayout->addWidget(pObjectiveTableYLabel, 6, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableYCurrentPos, 6, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableYTargetPos, 6, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableYSpeed, 6, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableYLeftBtn, 6, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableYRightBtn, 6, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableYHomeBtn, 6, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableYStopBtn, 6, 7, Qt::AlignCenter);

    // 设置间距
    pGridLayout->setSpacing(5);
    pMotionCtrlGroup->setLayout(pGridLayout);
    layout->addWidget(pMotionCtrlGroup);
}
