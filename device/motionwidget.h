#ifndef MOTIONWIDGET_H
#define MOTIONWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include "gycustomcontrol.h"

class QVBoxLayout;
class QTimer;
class QLabel;
class XraySource;
class QCheckBox;
class QPushButton;
class GongYTabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
    explicit GongYTabWidget(QWidget *parent = nullptr);
    ~GongYTabWidget();

    float opacity() const;
    void setOpacity(float value);

    void setSlashMark(bool slashMark);

private slots:
    void onCurrentChanged(int index);
    void slotXrayWarning();
    void slotGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua);
    void slotXonClick();
    void slotXoffClick();
    void slotVoltageChanged();
    void slotCurrentChanged();
    void slotAxisPosChanged();
    void slotAxisSpeedChanged();
private:
    void setupTabs();
    QWidget* createFirstTab();
    QWidget* createSecondTab();

    void createXrayGroup(QVBoxLayout* pGroupLayout);
    void createDetectorGroup(QVBoxLayout* pGroupLayout);
    void createAcquireGroup(QVBoxLayout* pGroupLayout);

    void setCheckboxStyle(QCheckBox* pCheckbox);

    float m_opacity;
    QFont m_font;
    QTimer* m_pTimer;
    QLabel* m_pLabel;
    bool m_slash;
    bool m_online;

    //射线源
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
    XraySource* m_pXraySource=nullptr;
};
#endif // MOTIONWIDGET_H
