#ifndef C3100AAXISDEF_H
#define C3100AAXISDEF_H

/*
 * @brief 3100A项目特定的轴ID定义
 * 从 MotionCtrl/motionctrlcommand.h 提取，用于UI和接口层解耦
 */
enum AXIS_3100A {
    objectiveTable_X1 = 0, // 载物台 X 轴
    objectiveTable_X2 = 1, // 载物台 X2 轴 (原UI隐藏)
    objectiveTable_Y1 = 2, // 载物台 Y 轴
    objectiveTable_Y2 = 3, // 载物台 Y2 轴 (原UI隐藏)
    Detector_Z1 = 4,       // 探测器 Z1 轴
    Detector_Z2 = 5,       // 探测器 Z2 轴 (原UI隐藏)
    XRAY_Z = 6,            // 射线源 Z 轴
    Detector_R = 7,        // 探测器 R 轴
    Detector_W = 8         // 探测器 W (theta) 轴
};

#endif // C3100AAXISDEF_H
