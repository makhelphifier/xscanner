#ifndef C3100AMOTIONWIDGET_H
#define C3100AMOTIONWIDGET_H

#include <QTabWidget>
#include <QMap>
#include <QFont>
#include "device_3100A/C3100AControls.h"

class IMotionController;
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QRadioButton;
class C3100AIntSpinBox;
class C3100ADoubleSpinBox;
class C3100APushButton;
class QPushButton;

class C3100AMotionWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit C3100AMotionWidget(IMotionController* controller, QWidget *parent = nullptr);
    ~C3100AMotionWidget();

public slots:
    void slot_DataChanged(const QVariant &var);

private slots:
    void slotAxisPosChanged();
    void slotAxisSpeedChanged();
    void slotLeftButtonPress();
    void slotRightButtonPress();
    void slotButtonRelease();
    void slotHomeButtonClick();
    void slotStopButtonClick();
    void slotStopAllBtnClick();

    void slotXonClick();
    void slotXoffClick();
    void slotVoltageChanged();
    void slotCurrentChanged();

private:
    void setupTabs();
    QWidget* createMotionTab();
    void createXrayGroup(QVBoxLayout* pGroupLayout);
    void createMotionControlGroup(QVBoxLayout* pGroupLayout);
    void createAcquireGroup(QVBoxLayout* pGroupLayout);
    void createAxisRow(QGridLayout* layout, int row, int axisId, const QString& name);

    IMotionController* m_controller;
    QFont m_font;

    QLineEdit* m_pVolCurrentLineEdit = nullptr;
    C3100AIntSpinBox * m_pVolTargetSpinbox = nullptr;
    QLineEdit* m_pCurrentCurrentLineEdit = nullptr;
    C3100AIntSpinBox* m_pCurrentTargetSpinbox = nullptr;
    QCheckBox* m_pXray = nullptr;
    QCheckBox* m_pWarmUp = nullptr;
    QCheckBox* m_pInterLock = nullptr;
    QCheckBox* m_pConnect = nullptr;
    QPushButton* m_pButtonXon = nullptr;
    QPushButton* m_pButtonXoff = nullptr;

    QMap<int, C3100ADoubleSpinBox*> m_currentPosWidgets;
    QMap<int, C3100ADoubleSpinBox*> m_targetPosWidgets;
    QMap<int, C3100ADoubleSpinBox*> m_speedWidgets;

    QRadioButton* m_pStepRadioBtn = nullptr;
    QRadioButton* m_pContineRadioBtn = nullptr;
    QRadioButton* m_pDrRadioBtn = nullptr;
    QRadioButton* m_pSpiralRadioBtn = nullptr;
    QRadioButton* m_pBiasRadioBtn = nullptr;
};

#endif // C3100AMOTIONWIDGET_H
