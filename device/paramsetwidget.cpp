#include "paramsetwidget.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>

#include "motionconfig.h"
#include "motioncontroller.h"
#include "xraysource.h"

static const QString WORK_DIR = "NDT1717HS";

BaseWidget::BaseWidget(QWidget* parent):QWidget(parent),m_font("Microsoft YaHei")
{

}

BaseWidget::~BaseWidget()
{
}

DetectorWidget::DetectorWidget(QWidget* parent):BaseWidget(parent),m_isAcquiring(false),m_isDetectorConnected(false)
{
    initUI();

    m_gainSlider->setEnabled(false);

    m_detectorController = DetectorController::getInstance();
    connect(m_detectorController, &DetectorController::sig_didConnect, this, [this](int state){
        m_isDetectorConnected = (state == 1);
        // addDetectorLogEntry(QString("探测器连接%1").arg(state == 1 ? "成功" : "失败"), "info");

        // Update status indicator
        if (m_isDetectorConnected) {
            emit sig_detectorStatusChanged(true);
        } else {
            emit sig_detectorStatusChanged(false);
        }

        // Update controls enabled state
        updateControlsEnabledState();
    });

    connect(m_detectorController, &DetectorController::sig_didStartAcquiring, this, [this](int state){
        m_isAcquiring = true;

        // Update controls enabled state
        updateControlsEnabledState();


        // addDetectorLogEntry("开始图像采集", "success");
        QString pixelMode = m_pPixelComboBo->currentText();
        int frameRate = m_frameRateSpinBox->value();
        // updateDetectorPreviewStatus(QString("像素模式: %1 | 帧率: %2fps | 状态: 采集中")
        //                                 .arg(pixelMode).arg(frameRate));
    });
    connect(m_detectorController, &DetectorController::sig_didStopAcquiring, this, [this](int state){
        m_isAcquiring = false;

        // Update controls enabled state
        updateControlsEnabledState();

        // addDetectorLogEntry("停止图像采集", "info");
        QString pixelMode = m_pPixelComboBo->currentText();
        // QString scanMode = m_scanModeCombo->currentText();
        int frameRate = m_frameRateSpinBox->value();
        // updateDetectorPreviewStatus(QString("像素模式: %1 | 帧率: %2fps | 状态: 待机")
        //                                 .arg(pixelMode).arg(frameRate));
    });
    connect(m_detectorController, &DetectorController::sig_didGetModeList, this, [this](const QVector<PixelModeInfo> pixelModeVec){
        m_pixelModeVec = pixelModeVec;
        m_pPixelComboBo->clear();
        for (int i = 0; i < m_pixelModeVec.count(); i++) {
            PixelModeInfo info = m_pixelModeVec.at(i);
            m_pPixelComboBo->addItem(QString("%1: Binning:%2, PGA:%3, Frequency:%4").arg(info.mode).arg(info.Binning).arg(info.PGA).arg(info.frequency));
        }
    });
    connect(m_detectorController, &DetectorController::sig_didStartCreateOffsetTemplate, this, [this](){
        // addDetectorLogEntry("开始更新Offset 模板，请不要操作射线源和探测器", "info");
    });

    connect(m_detectorController, &DetectorController::sig_didFinishCreateOffsetTemplate, this, [this](){
        // addDetectorLogEntry("更新Offset 模板完毕，可以打开射线源与探测器", "success");
    });
    connect(m_detectorController, &DetectorController::sig_didGetPreviewFramePixmap, [this](const QPixmap& pixmap){
        // addDetectorLogEntry("&DetectorViewModel::sig_didGetPreviewFramePixmap", "info");
        // m_previewDisplay->loadImage(pixmap);
        QLabel* pLabel = new QLabel();
        pLabel->setPixmap(pixmap);
        pLabel->showNormal();
    });


    QString workDirPath = QString("%1/%2").arg(qApp->applicationDirPath()).arg(WORK_DIR);
    m_detectorController->initWithWorkDir(workDirPath);

    // Set initial controls enabled state
    updateControlsEnabledState();
}

// void DetectorWidget::setPreviewImage(const QImage& image, bool windowLeveling)
// {
//     if (m_previewDisplay) {
//         // 使用延迟设置确保ImageGraphicsView已完全初始化
//         QTimer::singleShot(50, this, [this, image, windowLeveling]() {
//             // Convert QImage to QPixmap
//             QPixmap pixmap = QPixmap::fromImage(image);

//             // Clear existing scene or create new one
//             QGraphicsScene* scene = m_previewDisplay->scene();
//             if (!scene) {
//                 scene = new QGraphicsScene();
//                 m_previewDisplay->setScene(scene);
//             } else {
//                 scene->clear();
//             }

//             // Add pixmap to scene
//             scene->addPixmap(pixmap);
//             scene->setSceneRect(pixmap.rect());

//             // Set current image for info display
//             m_previewDisplay->setCurrentImage(pixmap);

//             // Fit the image in view
//             m_previewDisplay->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
//         });
//     }
// }

// void DetectorWidget::clearPreviewImage()
// {
//     if (m_previewDisplay) {
//         QPixmap emptyPixmap(512, 512);
//         emptyPixmap.fill(QColor(60, 79, 92)); // Same as background color

//         QGraphicsScene* scene = m_previewDisplay->scene();
//         if (!scene) {
//             scene = new QGraphicsScene();
//             m_previewDisplay->setScene(scene);
//         } else {
//             scene->clear();
//         }

//         scene->addPixmap(emptyPixmap);
//         scene->setSceneRect(emptyPixmap.rect());

//         // Clear current image for info display
//         m_previewDisplay->setCurrentImage(QPixmap());

//         m_previewDisplay->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
//     }
// }


DetectorWidget::~DetectorWidget()
{

}

void DetectorWidget::initUI()
{
    QVBoxLayout* layOut = new QVBoxLayout();
    this->setLayout(layOut);
    QGroupBox* pDetectorSettingGroup = new QGroupBox(tr("探测器设置"));
    pDetectorSettingGroup->setFont(m_font);
    pDetectorSettingGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px;}");
    layOut->addWidget(pDetectorSettingGroup);

    QGridLayout* pAttributeGridLayout = new QGridLayout();
    QVBoxLayout *pDetectorSetlayout = new QVBoxLayout(pDetectorSettingGroup);
    QHBoxLayout *pButsHayout = new QHBoxLayout();
    m_pStartAcqBtn = new QPushButton(tr("开始采集"));
    m_pStartAcqBtn->setFont(m_font);
    connect(m_pStartAcqBtn, &QPushButton::clicked, this, &DetectorWidget::slotStartAcquisition);
    m_pStopAcqBtn = new QPushButton(tr("停止采集"));
    m_pStopAcqBtn->setFont(m_font);
    m_pStopAcqBtn->setEnabled(false);
    connect(m_pStopAcqBtn, &QPushButton::clicked, this, &DetectorWidget::slotStopAcquisition);

    m_pStartAcqBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; border: none; padding: 8px 16px; border-radius: 2px; } "
                                            "QPushButton:hover { background-color: #218838; } "
                                            "QPushButton:disabled { background-color: #6c757d; }");
    m_pStopAcqBtn->setStyleSheet("QPushButton { background-color: #dc3545; color: white; border: none; padding: 8px 16px; border-radius: 2px; } "
                                           "QPushButton:hover { background-color: #c82333; } "
                                           "QPushButton:disabled { background-color: #6c757d; }");

    pButsHayout->addWidget(m_pStartAcqBtn);
    pButsHayout->addWidget(m_pStopAcqBtn);
    // pButsHayout->addStretch();
    pDetectorSetlayout->addLayout(pButsHayout);


    QLabel* pPixelLabel = new QLabel(tr("像素模式"));
    pPixelLabel->setAlignment(Qt::AlignLeft);
    pPixelLabel->setFont(m_font);
    m_pPixelComboBo = new QComboBox();
    m_pPixelComboBo->setFont(m_font);
    connect(m_pPixelComboBo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetectorWidget::slotPixelModeChanged);
    QLabel* pGainLabel = new QLabel(tr("增益"));
    pGainLabel->setAlignment(Qt::AlignLeft);
    pGainLabel->setFont(m_font);
    m_gainSlider = new QSlider(Qt::Horizontal);
    pGainLabel->setFont(m_font);
    m_gainSlider->setRange(1, 9);
    m_gainSlider->setValue(5); // Default value
    connect(m_gainSlider, &QSlider::valueChanged, this, &DetectorWidget::slotGainChanged);
    m_gainValueLabel = new QLabel("5");
    m_gainValueLabel->setAlignment(Qt::AlignCenter);
    m_gainValueLabel->setFont(m_font);
    m_gainValueLabel->setStyleSheet(
        "QLabel {"
        "    border: none;"
        "    color: #333333;"
        "    font-size: 12px;"
        "    min-width: 20px;"
        "    padding-left: 4px;"
        "}"
        );

    QLabel* pFrameRateLabel = new QLabel(tr("探测器帧率(fps):"));
    pFrameRateLabel->setFont(m_font);
    m_frameRateSpinBox = new QSpinBox();
    m_frameRateSpinBox->setRange(1, 60);
    m_frameRateSpinBox->setValue(15);
    m_frameRateSpinBox->setSuffix(" fps");
    m_frameRateSpinBox->setFont(m_font);
    connect(m_frameRateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DetectorWidget::slotFrameRateChanged);

    pAttributeGridLayout->addWidget(pPixelLabel,0,0);
    pAttributeGridLayout->addWidget(m_pPixelComboBo,0,1,1,2);
    pAttributeGridLayout->addWidget(pGainLabel,1,0);
    pAttributeGridLayout->addWidget(m_gainSlider,1,1);
    pAttributeGridLayout->addWidget(m_gainValueLabel,1,2);
    pAttributeGridLayout->addWidget(pFrameRateLabel,2,0);
    pAttributeGridLayout->addWidget(m_frameRateSpinBox,2,1,1,2);
    pDetectorSetlayout->addLayout(pAttributeGridLayout);


    QGroupBox* pAcqImgGroup = new QGroupBox(tr("获取图像"));
    pAcqImgGroup->setFont(m_font);
    pAcqImgGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px;}");
    layOut->addWidget(pAcqImgGroup);

    QGridLayout* pAcqGridLayout = new QGridLayout(pAcqImgGroup);
    QLabel* pImgCount = new QLabel(tr("图像数量:"));
    pImgCount->setFont(m_font);
    m_imageCountSpinBox = new QSpinBox();
    m_imageCountSpinBox->setFont(m_font);
    m_imageCountSpinBox->setRange(1, 10000);
    m_imageCountSpinBox->setValue(100);
    m_imageCountSpinBox->setSuffix(" 张");
    connect(m_imageCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DetectorWidget::slotImageCountChanged);
    m_stackImagesCheckBox = new QCheckBox(tr("图像叠加"));
    m_stackImagesCheckBox->setFont(m_font);
    m_stackImagesCheckBox = new QCheckBox("图像叠加");
    m_stackImagesCheckBox->setStyleSheet("QCheckBox { border: none; }");
    m_stackImagesCheckBox->setChecked(false);
    connect(m_stackImagesCheckBox, &QCheckBox::toggled, this, &DetectorWidget::slotStackImagesToggled);
    QPushButton* m_pAcqureImg = new QPushButton(tr("获取图像"));
    m_pAcqureImg->setStyleSheet("QPushButton { background-color: #007bff; color: white; border: none; padding: 8px 16px; border-radius: 2px; } "
                                   "QPushButton:hover { background-color: #0056b3; } "
                                   "QPushButton:disabled { background-color: #6c757d; }");
    connect(m_pAcqureImg, &QPushButton::clicked, this, &DetectorWidget::slotCaptureImages);
    m_pAcqureImg->setFont(m_font);
    pAcqGridLayout->addWidget(pImgCount,0,0);
    pAcqGridLayout->addWidget(m_imageCountSpinBox,0,1);
    pAcqGridLayout->addWidget(m_stackImagesCheckBox,1,0,1,2);
    pAcqGridLayout->addWidget(m_pAcqureImg,2,0,1,2);
}

void DetectorWidget::updateControlsEnabledState()
{
    // 开始采集按钮：只有在探测器连接且未采集时才启用
    bool startButtonEnabled = m_isDetectorConnected && !m_isAcquiring;
    m_pStartAcqBtn->setEnabled(startButtonEnabled);

    // 停止采集按钮：只有在采集中时才启用
    m_pStopAcqBtn->setEnabled(m_isAcquiring);

    // 像素模式控件和帧率控件：
    // - 探测器未连接时：disable
    // - 开始采集按钮 disable 时（探测器未连接）：disable
    // - 开始采集按钮 enabled 时（探测器已连接且未采集）：enable
    // - 采集中时：disable
    bool controlsEnabled = m_isDetectorConnected && !m_isAcquiring;
    m_pPixelComboBo->setEnabled(controlsEnabled);
    m_frameRateSpinBox->setEnabled(controlsEnabled);
}

void DetectorWidget::slotStartAcquisition()
{
    m_detectorController->startAcquire();
}

void DetectorWidget::slotStopAcquisition()
{
    m_detectorController->stopAcquire();
}

void DetectorWidget::slotPixelModeChanged(int sel)
{
    QString pixelMode = m_pPixelComboBo->currentText();
    // addDetectorLogEntry(QString("像素模式已更改为: %1").arg(pixelMode), "info");

    if (!m_isAcquiring) {
        // updateDetectorPreviewStatus(pixelMode);
        PixelModeInfo info = m_pixelModeVec.at(m_pPixelComboBo->currentIndex());
        m_gainSlider->setValue(info.PGA);
        m_frameRateSpinBox->setValue(info.frequency);

        m_detectorController->setMode(info.mode);
    }
}

void DetectorWidget::slotGainChanged(int val)
{
    int gainValue = m_gainSlider->value();
    m_gainValueLabel->setText(QString::number(gainValue));
    // addDetectorLogEntry(QString("增益已更改为: %1").arg(gainValue), "info");

    // Update preview status if not acquiring
    if (!m_isAcquiring) {
        QString pixelMode = m_pPixelComboBo->currentText();
        int frameRate = m_frameRateSpinBox->value();
        // updateDetectorPreviewStatus(QString("像素模式: %1 | 帧率: %2fps | 增益: %3 | 状态: 待机")
        //                                 .arg(pixelMode).arg(frameRate).arg(gainValue));
    }
}

void DetectorWidget::slotFrameRateChanged(int value)
{
    int frameRate = m_frameRateSpinBox->value();
    // addDetectorLogEntry(QString("帧率已更改为: %1fps").arg(frameRate), "info");

    if (!m_isAcquiring) {
        QString pixelMode = m_pPixelComboBo->currentText();
        // updateDetectorPreviewStatus(QString("像素模式: %1 | 帧率: %2fps | 状态: 待机")
        //                                 .arg(pixelMode).arg(frameRate));
    }
}

void DetectorWidget::slotImageCountChanged()
{
    // int imageCount = m_imageCountSpinBox->value();
    // addDetectorLogEntry(QString("图像数量设置为: %1张").arg(imageCount), "info");
}

void DetectorWidget::slotStackImagesToggled(bool checked)
{
    QString status = checked ? "启用" : "禁用";
    // addDetectorLogEntry(QString("图像叠加已%1").arg(status), "info");

    // if (checked) {
    //     addDetectorLogEntry("注意: 图像叠加将增加处理时间", "warning");
    // }
}

void DetectorWidget::slotCaptureImages()
{
    int imageCount = m_imageCountSpinBox->value();
    bool stackImages = m_stackImagesCheckBox->isChecked();

    QString savePath;

    if (stackImages) {
        // 如果stackImages为true，选择文件路径和文件名
        QString defaultFileName = QString("stacked_image_%1.raw")
                                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QString defaultPath = QDir(defaultDir).filePath(defaultFileName);

        savePath = QFileDialog::getSaveFileName(
            this,
            "选择保存叠加图像的文件路径",
            defaultPath,
            "图像文件 (*.raw);;所有文件 (*.*)"
            );

        if (savePath.isEmpty()) {
            // 用户取消了文件选择
            // addDetectorLogEntry("用户取消了图像获取操作", "warning");
            return;
        }

        // addDetectorLogEntry(QString("叠加图像将保存到: %1").arg(savePath), "info");

        m_detectorController->setSaveProjection(true, "", savePath, 1);
    }
    else {
        // 如果stackImages为false，选择文件夹
        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

        savePath = QFileDialog::getExistingDirectory(
            this,
            "选择保存图像序列的文件夹",
            defaultDir,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
            );

        if (savePath.isEmpty()) {
            // 用户取消了文件夹选择
            // addDetectorLogEntry("用户取消了图像获取操作", "warning");
            return;
        }

        // addDetectorLogEntry(QString("图像序列将保存到文件夹: %1").arg(savePath), "info");

        m_detectorController->setSaveProjection(true, savePath, "", m_imageCountSpinBox->value());
    }



    bool staticAcquiring = true;


    // 目前先调用原有方法，后续需要扩展ViewModel接口来接收保存路径
    m_detectorController->startGetMultiFrames(imageCount, (stackImages ? imageCount : 1), staticAcquiring);
}


MotorWidget::MotorWidget(QWidget* parent):BaseWidget(parent)
{
    initUI();

    m_pMotionController = new MotionController(this);
    connect(m_pMotionController,&MotionController::sig_motionMsg,this,[this](QString msg){
        // ui->textEdit->append(msg);
    });
    connect(m_pMotionController,&MotionController::sig_motionMsg,this,[this](QString msg){
        // ui->textEdit->append(msg);
    });
    connect(m_pMotionController,&MotionController::sig_dataChanged,this,&MotorWidget::slot_DataChanged);

    updateSpinBoxValue();
}

MotorWidget::~MotorWidget()
{
    if(m_pMotionController){
        m_pMotionController->deleteLater();
        m_pMotionController=nullptr;
    }

}

void MotorWidget::updateSpinBoxValue()
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

void MotorWidget::slot_DataChanged(const QVariant &var)
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

void MotorWidget::initUI()
{
    QVBoxLayout* layout = new QVBoxLayout();
    this->setLayout(layout);
    QGroupBox* pMotionCtrlGroup = new QGroupBox(tr("运动控制"));
    pMotionCtrlGroup->setFont(m_font);
    pMotionCtrlGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    layout->addWidget(pMotionCtrlGroup);

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
    connect(pXrayStopAllBtn,&QPushButton::clicked,this,&MotorWidget::slotStopAllBtnClick);
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
    connect(m_pXrayTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pXrayTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pXrayTargetPos->setFont(m_font);
    m_pXraySpeed = new GyDoubleSpinBox(3,Xray_X);
    connect(m_pXraySpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisSpeedChanged);
    // connect(m_pXraySpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    m_pXraySpeed->setFont(m_font);
    GyPushButton* pXrayXLeftBtn = new GyPushButton(Xray_X,tr("向左"));/*向左*/
    // pXrayXLeftBtn->setIcon(QIcon(":/device\\img/motionleft"));
    connect(pXrayXLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pXrayXLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pXrayXLeftBtn->setFont(m_font);
    GyPushButton* pXrayXRightBtn = new GyPushButton(Xray_X,tr("向右"));//向右
    // pXrayXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pXrayXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pXrayXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pXrayXRightBtn->setFont(m_font);
    GyPushButton* pXrayXHomeBtn = new GyPushButton(Xray_X,tr("回零"));//
    //pXrayXHomeBtn->setIcon(QIcon(":/img/home.png"));
    connect(pXrayXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pXrayXHomeBtn->setFont(m_font);
    GyPushButton* pXrayStopBtn = new GyPushButton(Xray_X, tr("停止"));//停止
    // pXrayStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pXrayStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pXrayZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pXrayZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pXrayZSpeed = new GyDoubleSpinBox(6,Xray_Z);
    m_pXrayZSpeed->setFont(m_font);
    connect(m_pXrayZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisSpeedChanged);
    // connect(m_pXrayZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pXrayZLeftBtn = new GyPushButton(Xray_Z,tr("向左"));
    pXrayZLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pXrayZLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pXrayZLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pXrayZLeftBtn->setFont(m_font);
    GyPushButton* pXrayZXRightBtn = new GyPushButton(Xray_Z,tr("向右"));
    pXrayZXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pXrayZXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pXrayZXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pXrayZXRightBtn->setFont(m_font);
    GyPushButton* pXrayZXHomeBtn = new GyPushButton(Xray_Z,tr("回零"));
    connect(pXrayZXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pXrayZXHomeBtn->setFont(m_font);

    GyPushButton* pXrayZStopBtn = new GyPushButton(Xray_Z,tr("停止"));
    pXrayZStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pXrayZStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pStageTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pStageTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageSpeed = new GyDoubleSpinBox(9,Material_X);
    m_pStageSpeed->setFont(m_font);
    connect(m_pStageSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisSpeedChanged);
    // connect(m_pStageSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pStageXLeftBtn = new GyPushButton(Material_X,tr("向左"));
    pStageXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pStageXLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pStageXLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pStageXLeftBtn->setFont(m_font);
    GyPushButton* pStageXRightBtn = new GyPushButton(Material_X,tr("向右"));
    pStageXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pStageXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pStageXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pStageXRightBtn->setFont(m_font);
    GyPushButton* pStageXHomeBtn = new GyPushButton(Material_X,tr("回零"));
    connect(pStageXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pStageXHomeBtn->setFont(m_font);

    GyPushButton* pStageStopBtn = new GyPushButton(Material_X,tr("停止"));
    pStageStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pStageStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pStageYTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pStageYTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageYSpeed = new GyDoubleSpinBox(12,Material_Y);
    m_pStageYSpeed->setFont(m_font);
    connect(m_pStageYSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisSpeedChanged);
    // connect(m_pStageYSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pStageYXLeftBtn = new GyPushButton(Material_Y,tr("向左"));
    pStageYXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pStageYXLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pStageYXLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pStageYXLeftBtn->setFont(m_font);
    GyPushButton* pStageYXRightBtn = new GyPushButton(Material_Y,tr("向右"));
    pStageYXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pStageYXRightBtn,&GyPushButton::pressed,this,&::MotorWidget::slotRightButtonPress);
    connect(pStageYXRightBtn,&GyPushButton::released,this,&::MotorWidget::slotButtonRelease);
    pStageYXRightBtn->setFont(m_font);
    GyPushButton* pStageYXHomeBtn = new GyPushButton(Material_Y,tr("回零"));
    connect(pStageYXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pStageYXHomeBtn->setFont(m_font);

    GyPushButton* pStageYStopBtn = new GyPushButton(Material_Y,tr("停止"));
    pStageYStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pStageYStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pStageZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pStageZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pStageZSpeed = new GyDoubleSpinBox(15,Material_Z);
    m_pStageZSpeed->setFont(m_font);
    connect(m_pStageZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisSpeedChanged);
    // connect(m_pStageZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pStageZLeftBtn = new GyPushButton(Material_Z,tr("向左"));
    pStageZLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pStageZLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pStageZLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pStageZLeftBtn->setFont(m_font);
    GyPushButton* pStageZXRightBtn = new GyPushButton(Material_Z,tr("向右"));
    pStageZXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pStageZXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pStageZXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pStageZXRightBtn->setFont(m_font);
    GyPushButton* pStageZXHomeBtn = new GyPushButton(Material_Z,tr("回零"));
    connect(pStageZXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pStageZXHomeBtn->setFont(m_font);

    GyPushButton* pStageZStopBtn = new GyPushButton(Material_Z,tr("停止"));
    pStageZStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pStageZStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pDetectorTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pDetectorTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorSpeed = new GyDoubleSpinBox(18,Detector_X);
    m_pDetectorSpeed->setFont(m_font);
    connect(m_pDetectorSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisSpeedChanged);
    // connect(m_pDetectorSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisSpeedChanged);
    GyPushButton* pDetectorXLeftBtn = new GyPushButton(Detector_X,tr("向左"));
    pDetectorXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pDetectorXLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pDetectorXLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pDetectorXLeftBtn->setFont(m_font);
    GyPushButton* pDetectorXRightBtn = new GyPushButton(Detector_X,tr("向右"));
    pDetectorXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pDetectorXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pDetectorXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pDetectorXRightBtn->setFont(m_font);
    GyPushButton* pDetectorXHomeBtn = new GyPushButton(Detector_X,tr("回零"));
    connect(pDetectorXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pDetectorXHomeBtn->setFont(m_font);
    GyPushButton* pDetectorStopBtn = new GyPushButton(Detector_X,tr("停止"));
    pDetectorStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pDetectorStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pDetectorYTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pDetectorYTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorYSpeed = new GyDoubleSpinBox(21,Detector_Y);
    m_pDetectorYSpeed->setFont(m_font);
    connect(m_pDetectorYSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pDetectorYSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    GyPushButton* pDetectorYXLeftBtn = new GyPushButton(Detector_Y,tr("向左"));
    pDetectorYXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pDetectorYXLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pDetectorYXLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pDetectorYXLeftBtn->setFont(m_font);
    GyPushButton* pDetectorYXRightBtn = new GyPushButton(Detector_Y,tr("向右"));
    pDetectorYXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pDetectorYXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pDetectorYXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pDetectorYXRightBtn->setFont(m_font);
    GyPushButton* pDetectorYXHomeBtn = new GyPushButton(Detector_Y,tr("回零"));
    connect(pDetectorYXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pDetectorYXHomeBtn->setFont(m_font);

    GyPushButton* pDetectorYStopBtn = new GyPushButton(Detector_Y,tr("停止"));
    pDetectorYStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pDetectorYStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
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
    connect(m_pDetectorZTargetPos->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pDetectorZTargetPos->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    m_pDetectorZSpeed = new GyDoubleSpinBox(24,Detector_Z);
    m_pDetectorZSpeed->setFont(m_font);
    connect(m_pDetectorZSpeed->LineEdit(),&QLineEdit::returnPressed,this,&MotorWidget::slotAxisPosChanged);
    // connect(m_pDetectorZSpeed->LineEdit(),&QLineEdit::editingFinished,this,&GongYTabWidget::slotAxisPosChanged);
    GyPushButton* pDetectorZXLeftBtn = new GyPushButton(Detector_Z,tr("向左"));
    pDetectorZXLeftBtn->setIcon(QIcon(":/img/left.png"));
    connect(pDetectorZXLeftBtn,&GyPushButton::pressed,this,&MotorWidget::slotLeftButtonPress);
    connect(pDetectorZXLeftBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pDetectorZXLeftBtn->setFont(m_font);
    GyPushButton* pDetectorZXRightBtn = new GyPushButton(Detector_Z,tr("向右"));
    pDetectorZXRightBtn->setIcon(QIcon(":/img/right.png"));
    connect(pDetectorZXRightBtn,&GyPushButton::pressed,this,&MotorWidget::slotRightButtonPress);
    connect(pDetectorZXRightBtn,&GyPushButton::released,this,&MotorWidget::slotButtonRelease);
    pDetectorZXRightBtn->setFont(m_font);
    GyPushButton* pDetectorZXHomeBtn = new GyPushButton(Detector_Z,tr("回零"));
    connect(pDetectorZXHomeBtn,&GyPushButton::clicked,this,&MotorWidget::slotHomeButtonClick);
    pDetectorZXHomeBtn->setFont(m_font);
    GyPushButton* pDetectorZStopBtn = new GyPushButton(Detector_Z,tr("停止"));
    pDetectorZStopBtn->setIcon(QIcon(":/img/stop.png"));
    connect(pDetectorZStopBtn,&GyPushButton::clicked,this,&MotorWidget::slotEndButtonClick);
    pDetectorZStopBtn->setFont(m_font);
    pGridLayout->addWidget(pDetectorZAixs,11,0,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZCurrentPos,11,1,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZTargetPos,11,2,Qt::AlignCenter);
    pGridLayout->addWidget(m_pDetectorZSpeed,11,3,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXLeftBtn,11,4,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXRightBtn,11,5,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZXHomeBtn,11,6,Qt::AlignCenter);
    pGridLayout->addWidget(pDetectorZStopBtn,11,7,Qt::AlignCenter);
}

void MotorWidget::slotStopAllBtnClick()
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

void MotorWidget::slotEndButtonClick()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisStopClicked(axis);
}

void MotorWidget::slotLeftButtonPress()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisLeftPressed(axis);
}

void MotorWidget::slotRightButtonPress()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisRightPressed(axis);
}

void MotorWidget::slotButtonRelease()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisStopClicked(axis);
}

void MotorWidget::slotHomeButtonClick()
{
    GyPushButton* pSendBtn = qobject_cast<GyPushButton*>(sender());
    if(pSendBtn == nullptr)
        return;

    AXIS axis = pSendBtn->Axis();
    m_pMotionController->onBtn_axisResetPosClicked(axis);
}

void MotorWidget::slotAxisPosChanged()
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

void MotorWidget::slotAxisSpeedChanged()
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

XrayWidget::XrayWidget(QWidget* parent):BaseWidget(parent)
{
    initUI();

    m_slash=false;
    m_pXraySource = XraySource::getInstance();
    connect(m_pXraySource, &XraySource::sig_didGetXrayStatus, this, &XrayWidget::slotGetXrayStatus);
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
    connect(m_pTimer,&QTimer::timeout,this,&XrayWidget::slotXrayWarning);
}

XrayWidget::~XrayWidget()
{
    if(m_pXraySource)
    {
        XraySource::destroyInstance();
        m_pXraySource=nullptr;
    }
}

void XrayWidget::initUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    this->setLayout(layout);
    // IO status list section
    QGroupBox* pXrayGroup = new QGroupBox(tr("射线机"));
    pXrayGroup->setFont(m_font);
    pXrayGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px;}");
    layout->addWidget(pXrayGroup);

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
    connect(m_pVolTargetSpinbox->LineEdit(),&QLineEdit::editingFinished,this,&XrayWidget::slotVoltageChanged);
    connect(m_pVolTargetSpinbox->LineEdit(),&QLineEdit::returnPressed,this,&XrayWidget::slotVoltageChanged);
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
    connect(m_pCurrentTargetSpinbox->LineEdit(),&QLineEdit::editingFinished,this,&XrayWidget::slotCurrentChanged);
    connect(m_pCurrentTargetSpinbox->LineEdit(),&QLineEdit::returnPressed,this,&XrayWidget::slotCurrentChanged);
    pcurrentFormLayout->addRow(tr("Current"),m_pCurrentCurrentLineEdit);
    pcurrentFormLayout->addRow(tr("Target"),m_pCurrentTargetSpinbox);
    pCurrentGroup->setLayout(pcurrentFormLayout);
    pVolCurrentHLayout->addWidget(pCurrentGroup);

    pXrayLayout->addLayout(pVolCurrentHLayout);
    // 添加按钮
    QHBoxLayout* pBtnsLayout = new QHBoxLayout();
    pBtnsLayout->addStretch();
    m_pButtonXon = new QPushButton(tr("XON"));
    connect(m_pButtonXon,&QPushButton::clicked,this,&XrayWidget::slotXonClick);
    pBtnsLayout->addWidget(m_pButtonXon);
    pBtnsLayout->addStretch();
    m_pButtonXoff = new QPushButton(tr("XOFF"));
    connect(m_pButtonXoff,&QPushButton::clicked,this,&XrayWidget::slotXoffClick);
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

}

void XrayWidget::setCheckboxStyle(QCheckBox* pCheckbox)
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

void XrayWidget::setSlashMark(bool slashMark)
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

void XrayWidget::slotXrayWarning()
{
    QString currentStyle = m_pLabel->styleSheet();
    if (currentStyle.contains("red")) {
        m_pLabel->setStyleSheet("background-color: white;");
    } else {
        m_pLabel->setStyleSheet("background-color: red;");
    }
}

void XrayWidget::slotGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua)
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


void XrayWidget::slotVoltageChanged()
{
    // if(m_online && m_pXraySource)
    // {
    //     m_pXraySource->setXrayKv(m_pVolTargetSpinbox->value());
    // }
}

void XrayWidget::slotCurrentChanged()
{
    // if(m_online && m_pXraySource)
    // {
    //     m_pXraySource->setXrayBeam(m_pCurrentTargetSpinbox->value());
    // }
}

void XrayWidget::slotXonClick()
{
    // if(!m_online)
    //     return;

    // m_pXraySource->setXrayOn();
}

void XrayWidget::slotXoffClick()
{
    // if(!m_online)
    //     return;

    // m_pXraySource->setXrayOff();
}
