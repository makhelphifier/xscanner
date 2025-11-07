#ifndef MOTIONCTRLVIEWMODEL_H
#define MOTIONCTRLVIEWMODEL_H

#include <QtCore>
#include <QObject>
#include <QPushButton>
#include <QMap>
#include "motionctrlmanager.h"

class MotionCtrlViewModel : public QObject
{
    Q_OBJECT
    friend class MotionCtrlWidget;

public:
    explicit MotionCtrlViewModel(QObject *parent = nullptr);

    QString loadDdSpeed(int index);
    QString loadAbsSpeed(int index);
    void saveSpeed(int index, QString ddSpeed, QString absSpeed);
    ~MotionCtrlViewModel();
public slots:
    void onDataChanged(const QVariant &var);


    //其它操作
    void onBtn_softResetClicked();

    void onBtn_boardResetClicked();

    void onBtn_openDoorClicked();
    void onBtn_closeDoorClicked();

    void onBtn_ClearAxisErrcode();//清除轴


    void onBtn_axisLeftPressed(int axis);//jog 左按钮 运动
    void onBtn_axisStopClicked(int axis);//jog 运动 停止
    void onBtn_axisRightPressed(int axis);//jog 右按钮 运动
    void onBtn_axisStartClicked(int axis,QString value);//绝对位置 开始运动
    void onBtn_axisResetPosClicked(int axis);//回零
    void onBtn_axisEnableClicked(int axis,bool status);//使能

    void onBtn_axisStartXYClicked( );//绝对位置 开始运动

signals:
    void sig_resetTime(int value);

    void sig_enableChanged(int index,bool status);

    void sig_beamContinuousScanning(double speed,double angleRange);

    void sig_posLimit();
private:

    void init();

    QString getValueFromIni(QString str);
    void setValue2Ini(QString str, QString value);
    void beamContinuousScanning();


    QString iniPath;

    QSharedPointer<QSettings> configSharedPointer;

    MotionCtrlManager *mcManger = nullptr;

    bool connected = false;

    double rayX_nLimit;
    double rayX_pLimit;
    double probeX_nLimit;
    double probeX_pLimit;
    double loaderY_nLimit;
    double loaderY_pLimit;
    double loaderZ_nLimit;
    double loaderZ_pLimit;

    QMap<QString,int> IoPortMap;

    QPushButton *doorStatusBtn;
};

#endif // MOTIONCTRLVIEWMODEL_H
