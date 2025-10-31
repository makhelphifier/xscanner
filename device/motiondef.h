#ifndef MOTIONDEF_H
#define MOTIONDEF_H
#include <QObject>

// #include "appconfig.h"
// #include "axisformula.h"
namespace MotionCtrlData {
struct MotionCtrlInfo
{
    bool enable;
    bool busy;
    bool org;
    bool almWarn;
    bool hardPosLmtWarn;
    bool hardNegLmtWarn;
    bool softPosLmtWarn;
    bool softNegLmtWarn;
    bool emgStop;
    double axEncPos;
    double axEncVel;
    float curdist;
    bool ZeroingIsComplete = false;//回零状态
    MotionCtrlInfo() {}

};

}Q_DECLARE_METATYPE(MotionCtrlData::MotionCtrlInfo);


enum MachineStatus
{
    MS_NONE,
    MS_WARN,
    MS_ALARM
};

// enum AXIS{
//     upper_material_X = 0,//进料X轴
//     upper_material_Y,//进料Y轴
//     upper_material_W,//载物台旋转W轴
//     Battery_material_W,//产品旋转W轴
//     annulus_W//圆环W轴
// };

enum AXIS{
    Xray_X=0,
    Xray_Z,
    Material_X,
    Material_Y,
    Material_Z,
    Detector_X,
    Detector_Y,
    Detector_Z
};


enum MotionDI{
    DI_stop = 11,
    DI_reset,
    DI_emgStop_1 = 14,
    DI_emgStop_2,
    DI_emgStop_3,
    DI_emgStop_4,
    DI_emgStop_5,
    DI_emgStop_6,
    DI_emgStop_7,
    DI_emgStop_8,
    DI_leftDoorBlot = 24,//左门插销信号
    DI_leftDoorLock,//左门上锁信号
    DI_rightDoorBlot,//右门插销信号
    DI_rightDoorLock,//右门上锁信号
    DI_backDoorBlot,//后门插销信号
    DI_backDoorLock,//后门上锁信号
    DI_Battery_test,//左上料模组Y轴载台电池检测
    DI_feed_test,//左上料模组Y轴载台检测
};

enum MotionDO{
    DO_xray = 26,
    DO_stop ,
    DO_reset,
    DO_leftDoorLock,
    DO_rightDoorLock,
    DO_backDoorLock,
    DO_red,
    DO_yellow,
    DO_green,//34
    DO_beep,//35
    DO_leftOuterExtends = 40,//左外移门气缸伸出
    DO_leftOutAnastole,//左外移门气缸缩回
    DO_rightOuterExtends,//右外移门气缸伸出
    DO_rightOutAnastole,//右外移门气缸缩回
    DO_leftInExtends ,//左内移门气缸伸出
    DO_leftInAnastole,//左内移门气缸缩回
    DO_rightInExtends,//右内移门气缸伸出
    DO_rightInAnastole,//右内移门气缸缩回

};


enum BatteryStatus
{
    BatteryNull=0,
    BatteryUpDet,//两侧都有料
    BatteryUp,//上料位 有料
    BatteryDet,//检测位有料
};

struct DIStatus
{
    ushort IO_Port;//端口
    ushort IO_Status;//状态
};
Q_DECLARE_METATYPE(DIStatus);

#endif // MOTIONDEF_H
