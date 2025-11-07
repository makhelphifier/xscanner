#ifndef APPCONFIG_H
#define APPCONFIG_H
#include <QtCore>
#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    #include <QtWidgets>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    #include <QtCore5Compat>
#endif
class AppConfig
{
public:
    static QString configFile;  //配置文件路径

    //读写配置参数
    void readConfig();           //读取配置参数
    void writeConfig();          //写入配置参数
    void setAxisPos(int value,float pos);
    float getAxisPos(int value);

    void setAxisLimitPos(int value,float pos);
    float getAxisLimitPos(int value);


    float getFodOffset();
    float getFddOffset();

    QString getNDTWorkDir();


    float getStartAxis0();
    float getStartAxis8();
    float getStartAxis_R();
    float getStartAxis_Y();

private:

    float axis1 = 10.0;
    float axis2 = 10.0;
    float axis3 = 10.0;
    float axis4 = 10.0;
    float axis5 = 10.0;
    float axis6 = 10.0;
    float axis7 = 10.0;
    float axis8 = 10.0;
    float axis9 = 10.0;
    float axis10 = 10.0;


    float axis1Limit = 10.0;
    float axis2Limit = 10.0;
    float axis3Limit = 10.0;
    float axis4Limit = 10.0;
    float axis5Limit = 10.0;
    float axis6Limit = 10.0;
    float axis7Limit = 10.0;
    float axis8Limit = 10.0;
    float axis9Limit = 10.0;
    float axis10Limit = 10.0;

    float fod = 45;
    float fdd = 766;

    float startAxis0 = 0;
    float startAxis8 = 0;
    float startAxis2 = 0;
    float startAxis7 = 0;

    QString NDTWorkdir = "/NDT1717HS_NM430001TA23240001X";

public:
    static float R ;
    static float totalLen ;
    static float sod ;
    static float d0 ;



    // --- 射线源Z轴 ---
    static float xrayZOffset;
    static float xrayZCoefficient;

    // --- 探测器Z1轴 ---
    static float detectorZOffset;
    static float detectorZCoefficient;

    // --- 探测器旋转R轴 ---
    static float detectorROffset;
    static float detectorRCoefficient;

    // --- 探测器旋转theta轴 ---
    static float detectorThetaOffset;
    static float detectorThetaCoefficient;

    // --- 载物台X轴 ---
    static float tableXOffset;
    static float tableXCoefficient;

    // --- 载物台Y轴 ---
    static float tableYOffset;
    static float tableYCoefficient;



};

#endif // APPCONFIG_H
