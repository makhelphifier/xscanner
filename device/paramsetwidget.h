#ifndef PARAMSETWIDGET_H
#define PARAMSETWIDGET_H

#include <QWidget>
#include "gycustomcontrol.h"
#include "detectorcontroller.h"
class QLabel;
class QCheckBox;
class QComboBox;
class QSlider;
class BaseWidget:public QWidget
{
    Q_OBJECT

public:
    BaseWidget(QWidget* parent=nullptr);
    virtual~BaseWidget();

protected:
    QFont m_font;
};

class DetectorController;
class DetectorWidget : public BaseWidget
{
    Q_OBJECT

public:
    DetectorWidget(QWidget* parent=nullptr);
    ~DetectorWidget();

private:
    void initUI();
    void updateControlsEnabledState();

    void setPreviewImage(const QImage& image, bool windowLeveling=false);
    void clearPreviewImage();

signals:
    void sig_detectorStatusChanged(bool status);

private slots:
    void slotStartAcquisition();
    void slotStopAcquisition();
    void slotPixelModeChanged(int sel);
    void slotGainChanged(int val);
    void slotFrameRateChanged(int val);
    void slotImageCountChanged();
    void slotStackImagesToggled(bool checked);
    void slotCaptureImages();


private:
    QPushButton* m_pStartAcqBtn = nullptr;
    QPushButton* m_pStopAcqBtn = nullptr;
    QComboBox* m_pPixelComboBo = nullptr;
    QSlider* m_gainSlider= nullptr;
    QLabel* m_gainValueLabel=nullptr;
    QSpinBox* m_frameRateSpinBox = nullptr;
    QSpinBox* m_imageCountSpinBox=nullptr;
    QCheckBox* m_stackImagesCheckBox=nullptr;
    QPushButton* m_pAcqureImg = nullptr;

    //state
    bool m_isAcquiring;
    bool m_isDetectorConnected;
    QVector<PixelModeInfo> m_pixelModeVec;

    DetectorController *m_detectorController;
};

class MotionController;
class MotorWidget : public BaseWidget
{
    Q_OBJECT

public:
    MotorWidget(QWidget* parent=nullptr);
    ~MotorWidget();

private slots:
    void slotLeftButtonPress();
    void slotRightButtonPress();
    void slotButtonRelease();
    void slotHomeButtonClick();
    void slotEndButtonClick();
    void slotAxisPosChanged();
    void slotAxisSpeedChanged();
    void slotStopAllBtnClick();
    void slot_DataChanged(const QVariant &var);
private:
    void initUI();
    void updateSpinBoxValue();

    //运动控制，射线源
    GyDoubleSpinBox* m_pXrayCurrentPos=nullptr;
    GyDoubleSpinBox* m_pXrayTargetPos=nullptr;
    GyDoubleSpinBox* m_pXraySpeed=nullptr;
    GyDoubleSpinBox* m_pXrayZCurrentPos=nullptr;
    GyDoubleSpinBox* m_pXrayZTargetPos=nullptr;
    GyDoubleSpinBox* m_pXrayZSpeed=nullptr;

    //运动控制，载物台x轴
    GyDoubleSpinBox* m_pStageCurrentPos=nullptr;
    GyDoubleSpinBox* m_pStageTargetPos = nullptr;
    GyDoubleSpinBox* m_pStageSpeed = nullptr;

    //运动控制，载物台y轴
    GyDoubleSpinBox* m_pStageYCurrentPos = nullptr;
    GyDoubleSpinBox* m_pStageYTargetPos = nullptr;
    GyDoubleSpinBox* m_pStageYSpeed = nullptr;

    //运动控制，载物台z轴
    GyDoubleSpinBox* m_pStageZCurrentPos = nullptr;
    GyDoubleSpinBox* m_pStageZTargetPos = nullptr;
    GyDoubleSpinBox* m_pStageZSpeed = nullptr;

    //运动控制，探测器x轴
    GyDoubleSpinBox* m_pDetectorCurrentPos = nullptr;
    GyDoubleSpinBox* m_pDetectorTargetPos = nullptr;
    GyDoubleSpinBox* m_pDetectorSpeed = nullptr;

    //运动控制，探测器y轴
    GyDoubleSpinBox* m_pDetectorYCurrentPos = nullptr;
    GyDoubleSpinBox* m_pDetectorYTargetPos = nullptr;
    GyDoubleSpinBox* m_pDetectorYSpeed = nullptr;

    //运动控制，探测器z轴
    GyDoubleSpinBox* m_pDetectorZCurrentPos = nullptr;
    GyDoubleSpinBox* m_pDetectorZTargetPos = nullptr;
    GyDoubleSpinBox* m_pDetectorZSpeed = nullptr;

    bool m_online;
    MotionController* m_pMotionController=nullptr;
};

class XraySource;
class XrayWidget : public BaseWidget
{
    Q_OBJECT

public:
    XrayWidget(QWidget* parent=nullptr);
    ~XrayWidget();

private slots:
    void slotVoltageChanged();
    void slotCurrentChanged();
    void slotXonClick();
    void slotXoffClick();
    void slotXrayWarning();
    void slotGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua);

private:
    void initUI();
    void setCheckboxStyle(QCheckBox* pCheckbox);
    void setSlashMark(bool slashMark);

    QLabel* m_pLabel;
    QLineEdit* m_pVolCurrentLineEdit = nullptr;
    GyIntSpinBox * m_pVolTargetSpinbox = nullptr;
    QLineEdit* m_pCurrentCurrentLineEdit = nullptr;
    GyIntSpinBox* m_pCurrentTargetSpinbox = nullptr;
    QCheckBox* m_pXray = nullptr;
    QCheckBox* m_pWarmUp = nullptr;
    QCheckBox* m_pInterLock = nullptr;
    QCheckBox* m_pConnect = nullptr;
    QPushButton* m_pButtonXon = nullptr;
    QPushButton* m_pButtonXoff = nullptr;
    bool m_online = true;
    QTimer* m_pTimer;
    bool m_slash;

    XraySource* m_pXraySource=nullptr;
};















#endif // PARAMSETWIDGET_H
