#ifndef C31__AMOTIONWIDGET_H
#define C31__AMOTIONWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include "C3100AControls.h"  // 自定义控件头文件，包含 C3100AIntSpinBox 和 C3100ADoubleSpinBox
#include <QObject>  // 添加：支持 Q_OBJECT 和 tr()

// 前向声明
class QVBoxLayout;
class QLabel;
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFormLayout;
class QGroupBox;
class QLabel;
class QCheckBox;
class QPushButton;

class C3100AMotionWidget : public QTabWidget
{
    Q_OBJECT  // 必须保留，支持 tr() 和信号槽
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
    explicit C3100AMotionWidget(QWidget *parent = nullptr);
    ~C3100AMotionWidget();

    float opacity() const;
    void setOpacity(float value);

    void setSlashMark(bool slashMark);

private:
    // UI 构建辅助方法（添加这些以匹配 .cpp 中的实现）
    void setupTabs();
    QWidget* createFirstTab();
    QWidget* createSecondTab();
    void createXrayGroup(QVBoxLayout* layout);  // 参数为 layout，与 .cpp 匹配
    void createDetectorGroup(QVBoxLayout* layout);  // 参数为 layout，与 .cpp 匹配

    // 射线源相关的 UI 控件（电压/电流/开关，非轴相关）
    QLineEdit* m_pVolCurrentLineEdit = nullptr;
    C3100AIntSpinBox* m_pVolTargetSpinbox = nullptr;
    QLineEdit* m_pCurrentCurrentLineEdit = nullptr;
    C3100AIntSpinBox* m_pCurrentTargetSpinbox = nullptr;
    QCheckBox* m_pXray = nullptr;
    QCheckBox* m_pWarmUp = nullptr;
    QCheckBox* m_pInterLock = nullptr;
    QCheckBox* m_pConnect = nullptr;
    QPushButton* m_pButtonXon = nullptr;
    QPushButton* m_pButtonXoff = nullptr;

    // 运动控制，射线源 Z 轴 (AXIS::XRAY_Z)
    C3100ADoubleSpinBox* m_pXrayZCurrentPos = nullptr;
    C3100ADoubleSpinBox* m_pXrayZTargetPos = nullptr;
    C3100ADoubleSpinBox* m_pXrayZSpeed = nullptr;

    // 运动控制，探测器 Z1 轴 (AXIS::Detector_Z1)
    C3100ADoubleSpinBox* m_pDetectorZ1CurrentPos = nullptr;
    C3100ADoubleSpinBox* m_pDetectorZ1TargetPos = nullptr;
    C3100ADoubleSpinBox* m_pDetectorZ1Speed = nullptr;

    // 运动控制，探测器 R 轴 (AXIS::Detector_R)
    C3100ADoubleSpinBox* m_pDetectorRCurrentPos = nullptr;
    C3100ADoubleSpinBox* m_pDetectorRTargetPos = nullptr;
    C3100ADoubleSpinBox* m_pDetectorRSpeed = nullptr;

    // 运动控制，探测器 theta 轴 (AXIS::Detector_Theta)
    C3100ADoubleSpinBox* m_pDetectorThetaCurrentPos = nullptr;
    C3100ADoubleSpinBox* m_pDetectorThetaTargetPos = nullptr;
    C3100ADoubleSpinBox* m_pDetectorThetaSpeed = nullptr;

    // 运动控制，载物台 X 轴 (AXIS::ObjectiveTable_X)
    C3100ADoubleSpinBox* m_pObjectiveTableXCurrentPos = nullptr;
    C3100ADoubleSpinBox* m_pObjectiveTableXTargetPos = nullptr;
    C3100ADoubleSpinBox* m_pObjectiveTableXSpeed = nullptr;

    // 运动控制，载物台 Y 轴 (AXIS::ObjectiveTable_Y)
    C3100ADoubleSpinBox* m_pObjectiveTableYCurrentPos = nullptr;
    C3100ADoubleSpinBox* m_pObjectiveTableYTargetPos = nullptr;
    C3100ADoubleSpinBox* m_pObjectiveTableYSpeed = nullptr;
};

#endif // C31__AMOTIONWIDGET_H
