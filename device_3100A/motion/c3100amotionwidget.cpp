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
#include <QDebug>
#include "util/logger/logger.h"
#include <QAxObject>
#include <QVariant>
#include "xcom_api.h"
C3100AMotionWidget::C3100AMotionWidget(QWidget *parent)
    : QTabWidget(parent),
    m_pTubeInterface(nullptr)
{
    qDebug()<<"sssssssss";
    m_pViewModel = new MotionCtrlViewModel(this); // 初始化 ViewModel
    m_appConfig.readConfig(); // 读取 AppConfig 配置文件

    setupTabs();
    // 设置默认 Tab
    setCurrentIndex(0);
    // 其他初始化（如 opacity）可在后续添加
    updateSpinBoxValue(); // 加载配置到 UI
    if (m_pViewModel && m_pViewModel->getMotionManager()) {
        connect(m_pViewModel->getMotionManager(), &MotionCtrlManager::sig_dataChanged, this, &C3100AMotionWidget::onAxisDataChanged);
    }


    qDebug() << "Simple Test: Initializing COM...";

    QAxObject* loader = new QAxObject("XRAYWorXBaseCOM.TubeLoaderCOM");
    if (loader->isNull()) {
        qWarning() << "Simple Test: Failed to create TubeLoaderCOM. Is DLL registered?";
        return;
    }

    QAxObject* defaultIp = loader->property("DefaultIpAddress").value<QAxObject*>();
    QString ip = defaultIp->property("Ip").toString();
    delete defaultIp;

    QVariant tubeInterfaceVariant = loader->dynamicCall("GetTubeInterface(QString)", ip);
    m_pTubeInterface = tubeInterfaceVariant.value<QAxObject*>();

    delete loader;

    if (!m_pTubeInterface || m_pTubeInterface->isNull()) {
        qWarning() << "Simple Test: Failed to get TubeInterface.";
        if(m_pTubeInterface) delete m_pTubeInterface;
        m_pTubeInterface = nullptr;
        return;
    }

    connect(m_pTubeInterface, SIGNAL(OnInitialized()), this, SLOT(onComInitialized()));

    if (m_pTubeInterface->property("IsInitialized").toBool()) {
        onComInitialized();
    }

}

C3100AMotionWidget::~C3100AMotionWidget()
{
    // 无需删除 UI 控件，Qt 父子关系自动管理
    delete m_pTubeInterface;
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
    connect(m_pButtonXon, &QPushButton::clicked, this, &C3100AMotionWidget::slotXonClick); // --- 使用这个    pBtnsLayout->addWidget(m_pButtonXon);
    pBtnsLayout->addStretch();
    m_pButtonXoff = new QPushButton(tr("XOFF"));
    // connect(m_pButtonXoff, &QPushButton::clicked, this, &C3100AMotionWidget::slotXoffClick);  // 移除槽
    pBtnsLayout->addWidget(m_pButtonXoff);
    pBtnsLayout->addStretch();
    pXrayLayout->addLayout(pBtnsLayout);
    m_pButtonXon->setEnabled(false);
    m_pButtonXoff->setEnabled(false);
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
    // --- “全部停止” 按钮连接 ---
    connect(pStopAllBtn, &QPushButton::clicked, this, [=](){
        for(int axis : {AXIS::XRAY_Z, AXIS::Detector_Z1, AXIS::Detector_R,
                         AXIS::Detector_W, AXIS::objectiveTable_X1, AXIS::objectiveTable_Y1})
        {
            if (m_pViewModel) {
                m_pViewModel->onBtn_axisStopClicked(axis);
            }
        }
    });
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
    m_pXrayZSpeed = new C3100ADoubleSpinBox();
    m_pXrayZSpeed->setRange(0, 100);  // 示例速度范围
    QPushButton* pXrayZLeftBtn = new QPushButton(tr("向左"));
    QPushButton* pXrayZRightBtn = new QPushButton(tr("向右"));
    QPushButton* pXrayZHomeBtn = new QPushButton(tr("回零"));
    QPushButton* pXrayZStopBtn = new QPushButton(tr("停止"));

    connectAxisControls(AXIS::XRAY_Z, m_pXrayZTargetPos, m_pXrayZSpeed,
                        pXrayZLeftBtn, pXrayZRightBtn, pXrayZHomeBtn, pXrayZStopBtn);

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

    connectAxisControls(AXIS::Detector_Z1, m_pDetectorZ1TargetPos, m_pDetectorZ1Speed,
                        pDetectorZ1LeftBtn, pDetectorZ1RightBtn, pDetectorZ1HomeBtn, pDetectorZ1StopBtn);
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

    connectAxisControls(AXIS::Detector_R, m_pDetectorRTargetPos, m_pDetectorRSpeed,
                        pDetectorRLeftBtn, pDetectorRRightBtn, pDetectorRHomeBtn, pDetectorRStopBtn);
    pGridLayout->addWidget(pDetectorRLabel, 3, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorRCurrentPos, 3, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorRTargetPos, 3, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorRSpeed, 3, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRLeftBtn, 3, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRRightBtn, 3, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRHomeBtn, 3, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorRStopBtn, 3, 7, Qt::AlignCenter);

    // 探测器 theta 轴 (AXIS::Detector_W) - 行 4
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

    connectAxisControls(AXIS::Detector_W, m_pDetectorThetaTargetPos, m_pDetectorThetaSpeed,
                        pDetectorThetaLeftBtn, pDetectorThetaRightBtn, pDetectorThetaHomeBtn, pDetectorThetaStopBtn);
    pGridLayout->addWidget(pDetectorThetaLabel, 4, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorThetaCurrentPos, 4, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorThetaTargetPos, 4, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorThetaSpeed, 4, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaLeftBtn, 4, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaRightBtn, 4, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaHomeBtn, 4, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorThetaStopBtn, 4, 7, Qt::AlignCenter);

    // 载物台 X 轴 (AXIS::objectiveTable_X1) - 行 5
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

    connectAxisControls(AXIS::objectiveTable_X1, m_pObjectiveTableXTargetPos, m_pObjectiveTableXSpeed,
                        pObjectiveTableXLeftBtn, pObjectiveTableXRightBtn, pObjectiveTableXHomeBtn, pObjectiveTableXStopBtn);
    pGridLayout->addWidget(pObjectiveTableXLabel, 5, 0, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableXCurrentPos, 5, 1, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableXTargetPos, 5, 2, Qt::AlignCenter);
    pGridLayout->addWidget(m_pObjectiveTableXSpeed, 5, 3, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXLeftBtn, 5, 4, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXRightBtn, 5, 5, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXHomeBtn, 5, 6, Qt::AlignCenter);
    pGridLayout->addWidget(pObjectiveTableXStopBtn, 5, 7, Qt::AlignCenter);

    // 载物台 Y 轴 (AXIS::objectiveTable_Y1) - 行 6
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

    connectAxisControls(AXIS::objectiveTable_Y1, m_pObjectiveTableYTargetPos, m_pObjectiveTableYSpeed,
                        pObjectiveTableYLeftBtn, pObjectiveTableYRightBtn, pObjectiveTableYHomeBtn, pObjectiveTableYStopBtn);

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


void C3100AMotionWidget::updateSpinBoxValue()
{
    if (!m_pViewModel) {
        qWarning("C3100AMotionWidget::updateSpinBoxValue() - m_pViewModel is null!");
        return;
    }

    m_pXrayZTargetPos->setValue(m_appConfig.getAxisPos(AXIS::XRAY_Z));
    m_pXrayZSpeed->setValue(m_pViewModel->loadAbsSpeed(AXIS::XRAY_Z).toDouble());

    m_pDetectorZ1TargetPos->setValue(m_appConfig.getAxisPos(AXIS::Detector_Z1));
    m_pDetectorZ1Speed->setValue(m_pViewModel->loadAbsSpeed(AXIS::Detector_Z1).toDouble());

    m_pDetectorRTargetPos->setValue(m_appConfig.getAxisPos(AXIS::Detector_R));
    m_pDetectorRSpeed->setValue(m_pViewModel->loadAbsSpeed(AXIS::Detector_R).toDouble());

    m_pDetectorThetaTargetPos->setValue(m_appConfig.getAxisPos(AXIS::Detector_W));
    m_pDetectorThetaSpeed->setValue(m_pViewModel->loadAbsSpeed(AXIS::Detector_W).toDouble());

    m_pObjectiveTableXTargetPos->setValue(m_appConfig.getAxisPos(AXIS::objectiveTable_X1));
    m_pObjectiveTableXSpeed->setValue(m_pViewModel->loadAbsSpeed(AXIS::objectiveTable_X1).toDouble());

    m_pObjectiveTableYTargetPos->setValue(m_appConfig.getAxisPos(AXIS::objectiveTable_Y1));
    m_pObjectiveTableYSpeed->setValue(m_pViewModel->loadAbsSpeed(AXIS::objectiveTable_Y1).toDouble());


}

void C3100AMotionWidget::onAxisDataChanged(const QVariant &var)
{

    QVector<MotionCtrlData::MotionCtrlInfo> mcVec = var.value<QVector<MotionCtrlData::MotionCtrlInfo>>();
    if(mcVec.size() < 9) {
        return;
    }
    // UI: 射线源 Z轴 (mm) -> AXIS::XRAY_Z (轴 6)
    m_pXrayZCurrentPos->setValue(mcVec[AXIS::XRAY_Z].axEncPos);
    // UI: 探测器 Z1轴 (mm) -> AXIS::Detector_Z1 (轴 4)
    m_pDetectorZ1CurrentPos->setValue(mcVec[AXIS::Detector_Z1].axEncPos);
    // UI: 探测器 R轴 (mm) -> AXIS::Detector_R (轴 7)
    m_pDetectorRCurrentPos->setValue(mcVec[AXIS::Detector_R].axEncPos);
    // UI: 探测器 theta轴 (°) -> AXIS::Detector_W (轴 8)
    m_pDetectorThetaCurrentPos->setValue(mcVec[AXIS::Detector_W].axEncPos);
    // UI: 载物台 X轴 (mm) -> AXIS::objectiveTable_X1 (轴 0)
    m_pObjectiveTableXCurrentPos->setValue(mcVec[AXIS::objectiveTable_X1].axEncPos);
    // UI: 载物台 Y轴 (mm) -> AXIS::objectiveTable_Y1 (轴 2)
    m_pObjectiveTableYCurrentPos->setValue(mcVec[AXIS::objectiveTable_Y1].axEncPos);
}


void C3100AMotionWidget::connectAxisControls(
    int axisId,
    C3100ADoubleSpinBox* targetPosSpin,
    C3100ADoubleSpinBox* speedSpin,
    QPushButton* leftBtn,
    QPushButton* rightBtn,
    QPushButton* homeBtn,
    QPushButton* stopBtn)
{
    if (!m_pViewModel) {
        qWarning("C3100AMotionWidget::connectAxisControls - m_pViewModel is null!");
        return;
    }

    connect(targetPosSpin, &C3100ADoubleSpinBox::editingFinished, this, [=](){
        double newPos = targetPosSpin->value();
        m_appConfig.setAxisPos(axisId, newPos);
        m_pViewModel->onBtn_axisStartClicked(axisId, targetPosSpin->text());
    });

    connect(speedSpin, &C3100ADoubleSpinBox::editingFinished, this, [=](){
        QString currentDdSpeed = m_pViewModel->loadAbsSpeed(axisId);
        QString newAbsSpeed = speedSpin->text();
        m_pViewModel->saveSpeed(axisId, currentDdSpeed, newAbsSpeed);

    });

    // Jog 左 (点按)
    connect(leftBtn, &QPushButton::pressed, this, [=](){ m_pViewModel->onBtn_axisLeftPressed(axisId); });
    connect(leftBtn, &QPushButton::released, this, [=](){ m_pViewModel->onBtn_axisStopClicked(axisId); });

    // Jog 右 (点按)
    connect(rightBtn, &QPushButton::pressed, this, [=](){ m_pViewModel->onBtn_axisRightPressed(axisId); });
    connect(rightBtn, &QPushButton::released, this, [=](){ m_pViewModel->onBtn_axisStopClicked(axisId); });

    // 回零 (点击)
    connect(homeBtn, &QPushButton::clicked, this, [=](){ m_pViewModel->onBtn_axisResetPosClicked(axisId); });

    // 停止 (点击)
    connect(stopBtn, &QPushButton::clicked, this, [=](){ m_pViewModel->onBtn_axisStopClicked(axisId); });

    Q_UNUSED(speedSpin); // 显式标记 speedSpin 暂时未使用
}


void C3100AMotionWidget::onComInitialized()
{
    qDebug() << "Simple Test: COM Interface Ready. Enabling buttons.";
    m_pButtonXon->setEnabled(true);
    m_pButtonXoff->setEnabled(true);
    m_pVolTargetSpinbox->setEnabled(true);
    m_pCurrentTargetSpinbox->setEnabled(true);
}

void C3100AMotionWidget::slotXonClick()
{
    if (!m_pTubeInterface) {
        qWarning() << "Simple Test: XON clicked, but m_pTubeInterface is null!";
        return;
    }

    // 对应手册 3.5: pTubeInterface->XRayOn->PcDemandValue = true;
    QAxObject* xrayOn = m_pTubeInterface->property("XRayOn").value<QAxObject*>();
    if (xrayOn) {
        qDebug() << "Simple Test: Sending X-Ray ON command...";
        xrayOn->setProperty("PcDemandValue", true);
        delete xrayOn; // 释放属性对象
    } else {
        qWarning() << "Simple Test: Failed to get 'XRayOn' property.";
    }
}
