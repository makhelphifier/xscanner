#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <QtCore>
#include <QObject>
#include <QPushButton>
#include <QMap>
#include "motionmanager.h"


class MotionController : public QObject
{
    Q_OBJECT
    friend class MotionCtrlWidget;

public:
    explicit MotionController(QObject *parent = nullptr);

    ~MotionController();
public slots:
    //其它操作
    void onBtn_softResetClicked();
    void onBtn_boardResetClicked();
    void onBtn_ClearAxisErrcode();//清除轴
    void onBtn_emgStop();
    void onBtn_axisLeftPressed(int axis);//jog 左按钮 运动
    void onBtn_axisStopClicked(int axis);//jog 运动 停止
    void onBtn_axisRightPressed(int axis);//jog 右按钮 运动
    void onBtn_axisStartClicked(int axis,QString value);//绝对位置 开始运动
    void onBtn_axisResetPosClicked(int axis);//回零
    void onBtn_axisEnableClicked(int axis,bool status);//使能

    void allaxisHoming();

    void setDoStatus(ushort bitNo);
    void writeOutbit(ushort bitNo, ushort out);

signals:
    void sig_dataChanged(const QVariant &var);//轴状态
    void sig_DIStatusdataChanged(const QVariant &var);
    void sig_resetTime(int value);
    void sig_enableChanged(int index,bool status);
    void sig_beamContinuousScanning(double degSpeed,double start,double angleRange,float step);
    void sig_motionConnect(bool );
    void sig_motionMsg(QString);

private:

    void init();

    bool m_connected = false;
    bool m_modelFlag = true;
    MotionManager* m_mManger = nullptr;
};

#endif // MOTIONCONTROLLER_H
