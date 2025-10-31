#include "motionconfig.h"
#include "motionhelper.h"

QString MotionConfig::motionConfigFile = "motionConfig.ini";

float MotionConfig::lastAxisPos[20] = {10};
float MotionConfig::lastAxisSpeed[20] = {10};
float MotionConfig::Left_feed_Y_bigPos = 70;//左上料模组Y上料位置
float MotionConfig::Left_feed_Y_smallPos = 5;//左上料模组Y下料位置

float MotionConfig::left_robot_Z_PickUpPos = 70;//左上料模组Y上料位置
float MotionConfig::left_robot_Z_TransitPos = 5;//左上料模组Y下料位置
float MotionConfig::left_robot_Z_BlankingPos = 70;//左上料模组Y上料位置

float MotionConfig::left_robotClamping1loosen = 5;//左上料模组Y下料位置
float MotionConfig::left_robotClamping1Clamping = 70;//左上料模组Y上料位置
float MotionConfig::left_robotClamping2loosen = 5;//左上料模组Y下料位置
float MotionConfig::left_robotClamping2Clamping = 70;//左上料模组Y上料位置


float MotionConfig::left_robot_down_1_down;//右上料模组Y下料位置
float MotionConfig::left_robot_down_1_up;//左上料模组Y上料位置
float MotionConfig::left_robot_down_2_down;//左上料模组Y下料位置
float MotionConfig::left_robot_down_2_up;//右上料模组Y上料位置


float MotionConfig::left_Feeding_X_upPos = 5;//左上料模组Y下料位置
float MotionConfig::left_Feeding_X_downPos = 5;//左上料模组X下料位置
float MotionConfig::left_Feeding_X_transferPos = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_X_detectPos1 = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_X_detectPos2 = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_X_detectPos3 = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_X_detectPos4 = 70;//左上料模组Y上料位置

float MotionConfig::left_Feeding_Y_upPos = 5;//左上料模组Y下料位置
float MotionConfig::left_Feeding_Y_downPos = 5;//左上料模组Y下料位置
float MotionConfig::left_Feeding_Y_transferPos = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_Y_detectPos1 = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_Y_detectPos2 = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_Y_detectPos3 = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_Y_detectPos4 = 70;//左上料模组Y上料位置


float MotionConfig::left_Feeding_W_up_pos = 5;//左上料模组Y下料位置
float MotionConfig::left_Feeding_W_1_pos = 5;//左上料模组Y下料位置
float MotionConfig::left_Feeding_W_2_pos = 70;//左上料模组Y上料位置
float MotionConfig::left_Feeding_W_3_pos = 5;//左上料模组Y下料位置
float MotionConfig::left_Feeding_W_4_pos = 5;//左上料模组Y下料位置

float MotionConfig::left_W_offset = 5;//左上料模组Y下料位置

float MotionConfig::Right_feed_Y_bigPos =70;//右上料模组Y上料位置
float MotionConfig::Right_feed_Y_smallPos = 5;//右上料模组Y下料位置

float MotionConfig::Right_robot_Z_PickUpPos = 70;//左上料模组Y上料位置
float MotionConfig::Right_robot_Z_TransitPos = 5;//左上料模组Y下料位置
float MotionConfig::Right_robot_Z_BlankingPos = 70;//左上料模组Y上料位置

float MotionConfig::Right_robotClamping1loosen = 5;//左上料模组Y下料位置
float MotionConfig::Right_robotClamping1Clamping = 70;//左上料模组Y上料位置
float MotionConfig::Right_robotClamping2loosen = 5;//左上料模组Y下料位置
float MotionConfig::Right_robotClamping2Clamping = 70;//左上料模组Y上料位置

float MotionConfig::Right_robot_down_1_down;//右上料模组Y下料位置
float MotionConfig::Right_robot_down_1_up;//左上料模组Y上料位置
float MotionConfig::Right_robot_down_2_down;//左上料模组Y下料位置
float MotionConfig::Right_robot_down_2_up;//右上料模组Y上料位置

float MotionConfig::Right_Feeding_X_upPos = 5;//左上料模组Y下料位置
float MotionConfig::Right_Feeding_X_downPos = 5;//左上料模组X下料位置
float MotionConfig::Right_Feeding_X_transferPos = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_X_detectPos1 = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_X_detectPos2 = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_X_detectPos3 = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_X_detectPos4 = 70;//左上料模组Y上料位置

float MotionConfig::Right_Feeding_Y_upPos = 5;//左上料模组Y下料位置
float MotionConfig::Right_Feeding_Y_downPos = 5;//左上料模组Y下料位置
float MotionConfig::Right_Feeding_Y_transferPos = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_Y_detectPos1 = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_Y_detectPos2 = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_Y_detectPos3 = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_Y_detectPos4 = 70;//左上料模组Y上料位置


float MotionConfig::Right_Feeding_W_up_pos = 5;//左上料模组Y下料位置
float MotionConfig::Right_Feeding_W_1_pos = 5;//左上料模组Y下料位置
float MotionConfig::Right_Feeding_W_2_pos = 70;//左上料模组Y上料位置
float MotionConfig::Right_Feeding_W_3_pos = 5;//左上料模组Y下料位置
float MotionConfig::Right_Feeding_W_4_pos = 5;//左上料模组Y下料位置
float MotionConfig::Right_W_offset = 5;//左上料模组Y下料位置

bool MotionConfig::EnableFourCorners = false;//启用4角
float MotionConfig::averageSpeedDist = 5;
void MotionConfig::readMotionConfig()
{
    QSettings set(MotionConfig::motionConfigFile, QSettings::IniFormat);

    //配置文件不存在或者不全则重新生成
    if (!MotionHelper::checkIniFile(MotionConfig::motionConfigFile))
    {
        writeMotionConfig();
        return;
    }

    //目标位置读取
    set.beginGroup("AxisPos");
    for(int i = 0;i<21;i++)
    {
        MotionConfig::lastAxisPos[i] = set.value(QString("lastAxisPos%1").arg(i)).toFloat();
    }
    set.endGroup();

    //目标速度读取
    set.beginGroup("AxisSpeed");
    for(int i = 0;i<21;i++)
    {
        MotionConfig::lastAxisSpeed[i] = set.value(QString("lastAxisSpeed%1").arg(i)).toFloat();
    }
    set.endGroup();

    //参数配置
    set.beginGroup("AxisParams");

    MotionConfig::averageSpeedDist = set.value("averageSpeedDist").toFloat();
 // qDebug()<<"MotionConfig::averageSpeedDist:"<<MotionConfig::averageSpeedDist;
    MotionConfig::Left_feed_Y_bigPos = set.value("Left_feed_Y_bigPos").toFloat();
    MotionConfig::Left_feed_Y_smallPos = set.value("Left_feed_Y_smallPos").toFloat();

    MotionConfig::left_robot_Z_PickUpPos = set.value("left_robot_Z_PickUpPos").toFloat();
    MotionConfig::left_robot_Z_TransitPos = set.value("left_robot_Z_TransitPos").toFloat();
    MotionConfig::left_robot_Z_BlankingPos = set.value("left_robot_Z_BlankingPos").toFloat();

    MotionConfig::left_robotClamping1loosen = set.value("left_robotClamping1loosen").toFloat();
    MotionConfig::left_robotClamping1Clamping = set.value("left_robotClamping1Clamping").toFloat();
    MotionConfig::left_robotClamping2loosen = set.value("left_robotClamping2loosen").toFloat();
    MotionConfig::left_robotClamping2Clamping = set.value("left_robotClamping2Clamping").toFloat();

    MotionConfig::left_robot_down_1_down = set.value("left_robot_down_1_down").toFloat();
    MotionConfig::left_robot_down_1_up = set.value("left_robot_down_1_up").toFloat();
    MotionConfig::left_robot_down_2_down = set.value("left_robot_down_2_down").toFloat();
    MotionConfig::left_robot_down_2_up = set.value("left_robot_down_2_up").toFloat();

    MotionConfig::left_Feeding_X_upPos = set.value("left_Feeding_X_upPos").toFloat();
    MotionConfig::left_Feeding_X_downPos = set.value("left_Feeding_X_downPos").toFloat();
    MotionConfig::left_Feeding_X_transferPos = set.value("left_Feeding_X_transferPos").toFloat();
    MotionConfig::left_Feeding_X_detectPos1 = set.value("left_Feeding_X_detectPos1").toFloat();
    MotionConfig::left_Feeding_X_detectPos2 = set.value("left_Feeding_X_detectPos2").toFloat();
    MotionConfig::left_Feeding_X_detectPos3 = set.value("left_Feeding_X_detectPos3").toFloat();
    MotionConfig::left_Feeding_X_detectPos4 = set.value("left_Feeding_X_detectPos4").toFloat();

    MotionConfig::left_Feeding_Y_upPos = set.value("left_Feeding_Y_upPos").toFloat();
    MotionConfig::left_Feeding_Y_downPos = set.value("left_Feeding_Y_downPos").toFloat();
    MotionConfig::left_Feeding_Y_transferPos = set.value("left_Feeding_Y_transferPos").toFloat();
    MotionConfig::left_Feeding_Y_detectPos1 = set.value("left_Feeding_Y_detectPos1").toFloat();
    MotionConfig::left_Feeding_Y_detectPos2 = set.value("left_Feeding_Y_detectPos2").toFloat();
    MotionConfig::left_Feeding_Y_detectPos3 = set.value("left_Feeding_Y_detectPos3").toFloat();
    MotionConfig::left_Feeding_Y_detectPos4 = set.value("left_Feeding_Y_detectPos4").toFloat();


    MotionConfig::left_Feeding_W_up_pos = set.value("left_Feeding_W_up_pos").toFloat();
    MotionConfig::left_Feeding_W_1_pos = set.value("left_Feeding_W_1_pos").toFloat();
    MotionConfig::left_Feeding_W_2_pos = set.value("left_Feeding_W_2_pos").toFloat();
    MotionConfig::left_Feeding_W_3_pos = set.value("left_Feeding_W_3_pos").toFloat();
    MotionConfig::left_Feeding_W_4_pos = set.value("left_Feeding_W_4_pos").toFloat();

    MotionConfig::left_W_offset = set.value("left_W_offset").toFloat();


    MotionConfig::Right_feed_Y_bigPos = set.value("Right_feed_Y_bigPos").toFloat();
    MotionConfig::Right_feed_Y_smallPos = set.value("Right_feed_Y_smallPos").toFloat();

    MotionConfig::Right_robot_Z_PickUpPos = set.value("Right_robot_Z_PickUpPos").toFloat();
    MotionConfig::Right_robot_Z_TransitPos = set.value("Right_robot_Z_TransitPos").toFloat();
    MotionConfig::Right_robot_Z_BlankingPos = set.value("Right_robot_Z_BlankingPos").toFloat();

    MotionConfig::Right_robotClamping1loosen = set.value("Right_robotClamping1loosen").toFloat();
    MotionConfig::Right_robotClamping1Clamping = set.value("Right_robotClamping1Clamping").toFloat();
    MotionConfig::Right_robotClamping2loosen = set.value("Right_robotClamping2loosen").toFloat();
    MotionConfig::Right_robotClamping2Clamping = set.value("Right_robotClamping2Clamping").toFloat();

    MotionConfig::Right_robot_down_1_down = set.value("Right_robot_down_1_down").toFloat();
    MotionConfig::Right_robot_down_1_up = set.value("Right_robot_down_1_up").toFloat();
    MotionConfig::Right_robot_down_2_down = set.value("Right_robot_down_2_down").toFloat();
    MotionConfig::Right_robot_down_2_up = set.value("Right_robot_down_2_up").toFloat();

    MotionConfig::Right_Feeding_X_upPos = set.value("Right_Feeding_X_upPos").toFloat();
    MotionConfig::Right_Feeding_X_downPos = set.value("Right_Feeding_X_downPos").toFloat();
    MotionConfig::Right_Feeding_X_transferPos = set.value("Right_Feeding_X_transferPos").toFloat();
    MotionConfig::Right_Feeding_X_detectPos1 = set.value("Right_Feeding_X_detectPos1").toFloat();
    MotionConfig::Right_Feeding_X_detectPos2 = set.value("Right_Feeding_X_detectPos2").toFloat();
    MotionConfig::Right_Feeding_X_detectPos3 = set.value("Right_Feeding_X_detectPos3").toFloat();
    MotionConfig::Right_Feeding_X_detectPos4 = set.value("Right_Feeding_X_detectPos4").toFloat();

    MotionConfig::Right_Feeding_Y_upPos = set.value("Right_Feeding_Y_upPos").toFloat();
    MotionConfig::Right_Feeding_Y_downPos = set.value("Right_Feeding_Y_downPos").toFloat();
    MotionConfig::Right_Feeding_Y_transferPos = set.value("Right_Feeding_Y_transferPos").toFloat();
    MotionConfig::Right_Feeding_Y_detectPos1 = set.value("Right_Feeding_Y_detectPos1").toFloat();
    MotionConfig::Right_Feeding_Y_detectPos2 = set.value("Right_Feeding_Y_detectPos2").toFloat();
    MotionConfig::Right_Feeding_Y_detectPos3 = set.value("Right_Feeding_Y_detectPos3").toFloat();
    MotionConfig::Right_Feeding_Y_detectPos4 = set.value("Right_Feeding_Y_detectPos4").toFloat();


    MotionConfig::Right_Feeding_W_up_pos = set.value("Right_Feeding_W_up_pos").toFloat();
    MotionConfig::Right_Feeding_W_1_pos = set.value("Right_Feeding_W_1_pos").toFloat();
    MotionConfig::Right_Feeding_W_2_pos = set.value("Right_Feeding_W_2_pos").toFloat();
    MotionConfig::Right_Feeding_W_3_pos = set.value("Right_Feeding_W_3_pos").toFloat();
    MotionConfig::Right_Feeding_W_4_pos = set.value("Right_Feeding_W_4_pos").toFloat();

    MotionConfig::Right_W_offset = set.value("Right_W_offset").toFloat();
    MotionConfig::EnableFourCorners = set.value("EnableFourCorners").toBool();

    set.endGroup();
}

void MotionConfig::writeMotionConfig()
{
    QSettings set(MotionConfig::motionConfigFile, QSettings::IniFormat);

    set.beginGroup("AxisPos");
    for(int i = 0;i<21;i++)
    {
        set.setValue(QString("lastAxisPos%1").arg(i), QString::number(MotionConfig::lastAxisPos[i],'f',5));
    }
    set.endGroup();

    //目标速度读取
    set.beginGroup("AxisSpeed");
    for(int i = 0;i<21;i++)
    {
        set.setValue(QString("lastAxisSpeed%1").arg(i), QString::number(MotionConfig::lastAxisSpeed[i],'f',5));
    }
    set.endGroup();



    //参数配置
    set.beginGroup("AxisParams");

    set.setValue("averageSpeedDist", QString::number(MotionConfig::averageSpeedDist,'f',5));


    set.setValue("Left_feed_Y_bigPos", QString::number(MotionConfig::Left_feed_Y_bigPos,'f',5));
    set.setValue("Left_feed_Y_smallPos", QString::number(MotionConfig::Left_feed_Y_smallPos,'f',5));

    set.setValue("left_robot_Z_PickUpPos", QString::number(MotionConfig::left_robot_Z_PickUpPos,'f',5));
    set.setValue("left_robot_Z_TransitPos", QString::number(MotionConfig::left_robot_Z_TransitPos,'f',5));
    set.setValue("left_robot_Z_BlankingPos", QString::number(MotionConfig::left_robot_Z_BlankingPos,'f',5));

    set.setValue("left_robotClamping1loosen", QString::number(MotionConfig::left_robotClamping1loosen,'f',5));
    set.setValue("left_robotClamping1Clamping", QString::number(MotionConfig::left_robotClamping1Clamping,'f',5));
    set.setValue("left_robotClamping2loosen", QString::number(MotionConfig::left_robotClamping2loosen,'f',5));
    set.setValue("left_robotClamping2Clamping", QString::number(MotionConfig::left_robotClamping2Clamping,'f',5));

    set.setValue("left_robot_down_1_down", QString::number(MotionConfig::left_robot_down_1_down,'f',5));
    set.setValue("left_robot_down_1_up", QString::number(MotionConfig::left_robot_down_1_up,'f',5));
    set.setValue("left_robot_down_2_down", QString::number(MotionConfig::left_robot_down_2_down,'f',5));
    set.setValue("left_robot_down_2_up", QString::number(MotionConfig::left_robot_down_2_up,'f',5));

    set.setValue("left_Feeding_X_upPos", QString::number(MotionConfig::left_Feeding_X_upPos,'f',5));
    set.setValue("left_Feeding_X_downPos", QString::number(MotionConfig::left_Feeding_X_downPos,'f',5));
    set.setValue("left_Feeding_X_transferPos", QString::number(MotionConfig::left_Feeding_X_transferPos,'f',5));
    set.setValue("left_Feeding_X_detectPos1", QString::number(MotionConfig::left_Feeding_X_detectPos1,'f',5));
    set.setValue("left_Feeding_X_detectPos2", QString::number(MotionConfig::left_Feeding_X_detectPos2,'f',5));
    set.setValue("left_Feeding_X_detectPos3", QString::number(MotionConfig::left_Feeding_X_detectPos3,'f',5));
    set.setValue("left_Feeding_X_detectPos4", QString::number(MotionConfig::left_Feeding_X_detectPos4,'f',5));

    set.setValue("left_Feeding_Y_upPos", QString::number(MotionConfig::left_Feeding_Y_upPos,'f',5));
    set.setValue("left_Feeding_Y_downPos", QString::number(MotionConfig::left_Feeding_Y_downPos,'f',5));

    set.setValue("left_Feeding_Y_transferPos", QString::number(MotionConfig::left_Feeding_Y_transferPos,'f',5));
    set.setValue("left_Feeding_Y_detectPos1", QString::number(MotionConfig::left_Feeding_Y_detectPos1,'f',5));
    set.setValue("left_Feeding_Y_detectPos2", QString::number(MotionConfig::left_Feeding_Y_detectPos2,'f',5));
    set.setValue("left_Feeding_Y_detectPos3", QString::number(MotionConfig::left_Feeding_Y_detectPos3,'f',5));
    set.setValue("left_Feeding_Y_detectPos4", QString::number(MotionConfig::left_Feeding_Y_detectPos4,'f',5));


    set.setValue("left_Feeding_W_up_pos", QString::number(MotionConfig::left_Feeding_W_up_pos,'f',5));
    set.setValue("left_Feeding_W_1_pos", QString::number(MotionConfig::left_Feeding_W_1_pos,'f',5));
    set.setValue("left_Feeding_W_2_pos", QString::number(MotionConfig::left_Feeding_W_2_pos,'f',5));
    set.setValue("left_Feeding_W_3_pos", QString::number(MotionConfig::left_Feeding_W_3_pos,'f',5));
    set.setValue("left_Feeding_W_4_pos", QString::number(MotionConfig::left_Feeding_W_4_pos,'f',5));

    set.setValue("left_W_offset", QString::number(MotionConfig::left_W_offset,'f',5));


    set.setValue("Right_feed_Y_bigPos", QString::number(MotionConfig::Right_feed_Y_bigPos,'f',5));
    set.setValue("Right_feed_Y_smallPos", QString::number(MotionConfig::Right_feed_Y_smallPos,'f',5));

    set.setValue("Right_robot_Z_PickUpPos", QString::number(MotionConfig::Right_robot_Z_PickUpPos,'f',5));
    set.setValue("Right_robot_Z_TransitPos", QString::number(MotionConfig::Right_robot_Z_TransitPos,'f',5));
    set.setValue("Right_robot_Z_BlankingPos", QString::number(MotionConfig::Right_robot_Z_BlankingPos,'f',5));

    set.setValue("Right_robotClamping1loosen", QString::number(MotionConfig::Right_robotClamping1loosen,'f',5));
    set.setValue("Right_robotClamping1Clamping", QString::number(MotionConfig::Right_robotClamping1Clamping,'f',5));
    set.setValue("Right_robotClamping2loosen", QString::number(MotionConfig::Right_robotClamping2loosen,'f',5));
    set.setValue("Right_robotClamping2Clamping", QString::number(MotionConfig::Right_robotClamping2Clamping,'f',5));


    set.setValue("Right_robot_down_1_down", QString::number(MotionConfig::Right_robot_down_1_down,'f',5));
    set.setValue("Right_robot_down_1_up", QString::number(MotionConfig::Right_robot_down_1_up,'f',5));
    set.setValue("Right_robot_down_2_down", QString::number(MotionConfig::Right_robot_down_2_down,'f',5));
    set.setValue("Right_robot_down_2_up", QString::number(MotionConfig::Right_robot_down_2_up,'f',5));


    set.setValue("Right_Feeding_X_upPos", QString::number(MotionConfig::Right_Feeding_X_upPos,'f',5));
    set.setValue("Right_Feeding_X_downPos", QString::number(MotionConfig::Right_Feeding_X_downPos,'f',5));
    set.setValue("Right_Feeding_X_transferPos", QString::number(MotionConfig::Right_Feeding_X_transferPos,'f',5));
    set.setValue("Right_Feeding_X_detectPos1", QString::number(MotionConfig::Right_Feeding_X_detectPos1,'f',5));
    set.setValue("Right_Feeding_X_detectPos2", QString::number(MotionConfig::Right_Feeding_X_detectPos2,'f',5));
    set.setValue("Right_Feeding_X_detectPos3", QString::number(MotionConfig::Right_Feeding_X_detectPos3,'f',5));
    set.setValue("Right_Feeding_X_detectPos4", QString::number(MotionConfig::Right_Feeding_X_detectPos4,'f',5));

    set.setValue("Right_Feeding_Y_upPos", QString::number(MotionConfig::Right_Feeding_Y_upPos,'f',5));
    set.setValue("Right_Feeding_Y_downPos", QString::number(MotionConfig::Right_Feeding_Y_downPos,'f',5));

    set.setValue("Right_Feeding_Y_transferPos", QString::number(MotionConfig::Right_Feeding_Y_transferPos,'f',5));
    set.setValue("Right_Feeding_Y_detectPos1", QString::number(MotionConfig::Right_Feeding_Y_detectPos1,'f',5));
    set.setValue("Right_Feeding_Y_detectPos2", QString::number(MotionConfig::Right_Feeding_Y_detectPos2,'f',5));
    set.setValue("Right_Feeding_Y_detectPos3", QString::number(MotionConfig::Right_Feeding_Y_detectPos3,'f',5));
    set.setValue("Right_Feeding_Y_detectPos4", QString::number(MotionConfig::Right_Feeding_Y_detectPos4,'f',5));

    set.setValue("Right_Feeding_W_up_pos", QString::number(MotionConfig::Right_Feeding_W_up_pos,'f',5));
    set.setValue("Right_Feeding_W_1_pos", QString::number(MotionConfig::Right_Feeding_W_1_pos,'f',5));
    set.setValue("Right_Feeding_W_2_pos", QString::number(MotionConfig::Right_Feeding_W_2_pos,'f',5));
    set.setValue("Right_Feeding_W_3_pos", QString::number(MotionConfig::Right_Feeding_W_3_pos,'f',5));
    set.setValue("Right_Feeding_W_4_pos", QString::number(MotionConfig::Right_Feeding_W_4_pos,'f',5));

    set.setValue("Right_W_offset", QString::number(MotionConfig::Right_W_offset,'f',5));
    set.setValue("EnableFourCorners", MotionConfig::EnableFourCorners);
    set.endGroup();

}
