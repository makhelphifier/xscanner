#ifndef MOTIONCONFIG_H
#define MOTIONCONFIG_H
#include <QtCore>
#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    #include <QtWidgets>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    #include <QtCore5Compat>
#endif
class MotionConfig
{
public:

    static void readMotionConfig();           //读取配置参数
    static void writeMotionConfig();          //写入配置参数

public:
    static QString motionConfigFile;  //配置文件路径


    static float averageSpeedDist;//均速距离
    static float lastAxisPos[];//目标位置保存
    static float lastAxisSpeed[];//目标速度保存

    static float Left_feed_Y_bigPos;//左上料模组Y上料位置 （里侧是零点）
    static float Left_feed_Y_smallPos;//左上料模组Y下料位置

    static float left_robot_Z_PickUpPos;//左上料模组Z上料位置
    static float left_robot_Z_TransitPos;//左上料模组Z（下压步进）位置
    static float left_robot_Z_BlankingPos;//右上料模组Z下料位置

    static float left_robotClamping1loosen;//左上料模组夹爪松开位置
    static float left_robotClamping1Clamping;//左上料模组夹爪夹紧位置
    static float left_robotClamping2loosen;//左上料模组夹爪松开位置
    static float left_robotClamping2Clamping;//左上料模组夹爪夹紧位置


    static float left_robot_down_1_down;//左上料模组下压下压位置
    static float left_robot_down_1_up;//左上料模组下压抬起位置
    static float left_robot_down_2_down;//左上料模组下压下压位置
    static float left_robot_down_2_up;//左上料模组下压抬起位置


    static float left_Feeding_X_upPos;//左进料模组X上料位置
    static float left_Feeding_X_downPos ;//左进料模组X下料位置
    static float left_Feeding_X_transferPos;//左进料模组X中转位置
    static float left_Feeding_X_detectPos1;//左进料模组X检测位置
    static float left_Feeding_X_detectPos2;//左进料模组X检测位置
    static float left_Feeding_X_detectPos3;//左进料模组X检测位置
    static float left_Feeding_X_detectPos4;//左进料模组X检测位置

    static float left_Feeding_Y_upPos;//左进料模组Y上料位置
    static float left_Feeding_Y_downPos;//左进料模组Y下料位置
    static float left_Feeding_Y_transferPos;//左进料模组Y中转位置
    static float left_Feeding_Y_detectPos1;//左进上料模组Y检测位置
    static float left_Feeding_Y_detectPos2;//左进上料模组Y检测位置
    static float left_Feeding_Y_detectPos3;//左进上料模组Y检测位置
    static float left_Feeding_Y_detectPos4;//左进上料模组Y检测位置


    static float left_Feeding_W_up_pos;//右上料模组W上料位置
    static float left_Feeding_W_1_pos;//右上料模组Y下料位置
    static float left_Feeding_W_2_pos;//左上料模组Y上料位置
    static float left_Feeding_W_3_pos;//左上料模组Y下料位置
    static float left_Feeding_W_4_pos;//左上料模组Y下料位置

    static float left_W_offset;//左机械手W轴 起始位置


    static float Right_feed_Y_bigPos;//右上料模组Y上料位置
    static float Right_feed_Y_smallPos;//右上料模组Y下料位置

    static float Right_robot_Z_PickUpPos;//右上料模组Z上料位置
    static float Right_robot_Z_TransitPos;//右上料模组Z（下压步进）位置
    static float Right_robot_Z_BlankingPos;//右上料模组Z下料位置

    static float Right_robotClamping1loosen;//右上料模组夹爪松开位置
    static float Right_robotClamping1Clamping;//右上料模组夹爪夹紧位置
    static float Right_robotClamping2loosen;//右上料模组夹爪松开位置
    static float Right_robotClamping2Clamping;//右上料模组夹爪夹紧位置

    static float Right_robot_down_1_down;//右上料模组下压下压位置
    static float Right_robot_down_1_up;//右上料模组下压抬起位置
    static float Right_robot_down_2_down;//右上料模组下压下压位置
    static float Right_robot_down_2_up;//右上料模组下压抬起位置


    static float Right_Feeding_X_upPos;//右进料模组X上料位置
    static float Right_Feeding_X_downPos ;//右进料模组X下料位置
    static float Right_Feeding_X_transferPos;//右进料模组X中转位置
    static float Right_Feeding_X_detectPos1;//右进料模组X检测位置
    static float Right_Feeding_X_detectPos2;//右进料模组X检测位置
    static float Right_Feeding_X_detectPos3;//右进料模组X检测位置
    static float Right_Feeding_X_detectPos4;//右进料模组X检测位置

    static float Right_Feeding_Y_upPos;//右进料模组Y上料位置
    static float Right_Feeding_Y_downPos;//右进料模组Y上料位置
    static float Right_Feeding_Y_transferPos;//右进料模组Y中转位置
    static float Right_Feeding_Y_detectPos1;//右进料模组Y检测位置
    static float Right_Feeding_Y_detectPos2;//右进料模组Y检测位置
    static float Right_Feeding_Y_detectPos3;//右进料模组Y检测位置
    static float Right_Feeding_Y_detectPos4;//右进料模组Y检测位置


    static float Right_Feeding_W_up_pos;//右进料模组W下料位置
    static float Right_Feeding_W_1_pos;//右进料模组W检测位置
    static float Right_Feeding_W_2_pos;//右进料模组W检测位置
    static float Right_Feeding_W_3_pos;//右进料模组W检测位置
    static float Right_Feeding_W_4_pos;//右进料模组W检测位置

    static float Right_W_offset;//右机械手W轴 起始位置

    static bool EnableFourCorners;//启用4角

};

#endif // MOTIONCONFIG_H
