#ifndef CARDIO_H
#define CARDIO_H

#include <QWidget>
#include "motionctrlmanager.h"

namespace Ui {
class cardIO;
}
enum cardDI{
    XRAYCONDITIONSMET = 8,//停止按钮
    STOP = 9,//停止按钮
    RESET,//复位按钮
    EMGSTOP1=18,//急停按钮1
    EMGSTOP2,//急停按钮2
    DOORBOLT = 30,//铅门插销信号
    RIGHTDOORBOLT ,//背面门插销信号
    LEFTDOORBOLT,//左侧门插销信号
    DOORLOCK = 35,
    RIGHTDOORLOCK,//背面门上锁完成信号
    LEFTDOORLOCK,//左侧门上锁完成信号
};

enum cardDO{
    STOPBTN = 9,//停止按钮
    RESETBTN,//复位按钮
    XRAYLIGHT = 12,
    DOORLOCKBTN = 19,
    RIGHTDOORLOCKBTN,//背面门上锁完成信号
    LEFTDOORLOCKBTN,//左侧门上锁完成信号
    LIGHTRED = 28,//三色灯红
    LIGHTYELLOW,//三色灯黄
    LIGHTGREEN,//三色灯
    BUZZER,//蜂鸣器
    XRAYSAFETYCIRCUIT,//光管安全回路信号

};
class cardIO : public QWidget
{
    Q_OBJECT

public:
    explicit cardIO(QWidget *parent = nullptr);
    ~cardIO();

private slots:

    void on_btn_close_clicked();

    void slot_DataChanged(const QVariant &var);


private:
    Ui::cardIO *ui;

    bool backDoorLock = false;
    bool leftDoorLock = false;
    bool doorLock = false;


};

#endif // CARDIO_H
