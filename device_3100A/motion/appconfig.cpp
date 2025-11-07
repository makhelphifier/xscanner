#include "appconfig.h"
#include "apphelper.h"

QString AppConfig::configFile = "AxisPosconfig.ini";
float AppConfig::R = 277.3;
float AppConfig::totalLen = 590;
float AppConfig::d0 = 313.82;
float AppConfig::sod = 85;


float AppConfig::xrayZOffset = 0.0;
float AppConfig::xrayZCoefficient = 1.0;
float AppConfig::detectorZOffset = 0.0;
float AppConfig::detectorZCoefficient = 1.0;
float AppConfig::detectorROffset = 0.0;
float AppConfig::detectorRCoefficient = 1.0;
float AppConfig::detectorThetaOffset = 0.0;
float AppConfig::detectorThetaCoefficient = 1.0;
float AppConfig::tableXOffset = 0.0;
float AppConfig::tableXCoefficient = 1.0;
float AppConfig::tableYOffset = 0.0;
float AppConfig::tableYCoefficient = 1.0;

void AppConfig::readConfig()
{
    QSettings set(AppConfig::configFile, QSettings::IniFormat);

    //配置文件不存在或者不全则重新生成
    // if (!AppHelper::checkIniFile(AppConfig::configFile))
    // {
    //     writeConfig();
    //     return;
    // }

    set.beginGroup("Axis");
    axis1 = set.value("axis1").toFloat();
    axis2 = set.value("axis2").toFloat();
    axis3 = set.value("axis3").toFloat();
    axis4 = set.value("axis4").toFloat();
    axis5 = set.value("axis5").toFloat();
    axis6 = set.value("axis6").toFloat();
    axis7 = set.value("axis7").toFloat();
    axis8 = set.value("axis8").toFloat();
    axis9 = set.value("axis9").toFloat();
    axis10 = set.value("axis10").toFloat();

    set.endGroup();

    set.beginGroup("NDTWorkDir");

    NDTWorkdir = set.value("NDTWorkdir").toString();

    set.endGroup();


    set.beginGroup("ImagingAnchorParam");
    axis1Limit = set.value("axis1Limit").toFloat();
    axis2Limit = set.value("axis2Limit").toFloat();
    axis3Limit = set.value("axis3Limit").toFloat();
    axis4Limit = set.value("axis4Limit").toFloat();
    axis5Limit = set.value("axis5Limit").toFloat();
    axis6Limit = set.value("axis6Limit").toFloat();
    axis7Limit = set.value("axis7Limit").toFloat();
    axis8Limit = set.value("axis8Limit").toFloat();
    axis9Limit = set.value("axis9Limit").toFloat();
    axis10Limit = set.value("axis10Limit").toFloat();

    set.endGroup();


    set.beginGroup("Offset");
    fod = set.value("fodOffset").toFloat();
    fdd = set.value("fddOffset").toFloat();

    set.endGroup();



    set.beginGroup("StepParams");
    startAxis0 = set.value("startAxis0").toFloat();
    startAxis8 = set.value("startAxis8").toFloat();

    AppConfig::R = set.value("R").toFloat();
    AppConfig::totalLen = set.value("totalLen").toFloat();
    AppConfig::d0 = set.value("d0").toFloat();
    AppConfig::sod = set.value("sod").toFloat();

    qDebug()<<"~~~~!!!!!!startAxis0"<<startAxis0<<" startAxis8"<<startAxis8<<" r"<<AppConfig::R<<" totalLen"<<AppConfig::totalLen<<"  d0 ="<<AppConfig::d0;
    set.endGroup();


    // --- 新增：读取系数和补偿 ---
    set.beginGroup("AxisParams");
    xrayZOffset = set.value("xrayZOffset", 0.0).toFloat();
    xrayZCoefficient = set.value("xrayZCoefficient", 1.0).toFloat();
    detectorZOffset = set.value("detectorZOffset", 0.0).toFloat();
    detectorZCoefficient = set.value("detectorZCoefficient", 1.0).toFloat();
    detectorROffset = set.value("detectorROffset", 0.0).toFloat();
    detectorRCoefficient = set.value("detectorRCoefficient", 1.0).toFloat();
    detectorThetaOffset = set.value("detectorThetaOffset", 0.0).toFloat();
    detectorThetaCoefficient = set.value("detectorThetaCoefficient", 1.0).toFloat();
    tableXOffset = set.value("tableXOffset", 0.0).toFloat();
    tableXCoefficient = set.value("tableXCoefficient", 1.0).toFloat();
    tableYOffset = set.value("tableYOffset", 0.0).toFloat();
    tableYCoefficient = set.value("tableYCoefficient", 1.0).toFloat();
    set.endGroup();


}

void AppConfig::writeConfig()
{
    QSettings set(AppConfig::configFile, QSettings::IniFormat);

    set.beginGroup("Axis");
    set.setValue("axis1", QString::number(axis1,'f',5));
    set.setValue("axis2", QString::number(axis2,'f',5));
    set.setValue("axis3", QString::number(axis3,'f',5));
    set.setValue("axis4", QString::number(axis4,'f',5));
    set.setValue("axis5", QString::number(axis5,'f',5));
    set.setValue("axis6", QString::number(axis6,'f',5));
    set.setValue("axis7", QString::number(axis7,'f',5));
    set.setValue("axis8", QString::number(axis8,'f',5));
    set.setValue("axis9", QString::number(axis9,'f',5));
    set.setValue("axis10", QString::number(axis10,'f',5));
    set.endGroup();


    set.beginGroup("NDTWorkDir");

    set.setValue("NDTWorkdir",NDTWorkdir);

    set.endGroup();


    set.beginGroup("ImagingAnchorParam");
    set.setValue("axis1Limit", QString::number(axis1Limit,'f',5));
    set.setValue("axis2Limit", QString::number(axis2Limit,'f',5));
    set.setValue("axis3Limit", QString::number(axis3Limit,'f',5));
    set.setValue("axis4Limit", QString::number(axis4Limit,'f',5));
    set.setValue("axis5Limit", QString::number(axis5Limit,'f',5));
    set.setValue("axis6Limit", QString::number(axis6Limit,'f',5));
    set.setValue("axis7Limit", QString::number(axis7Limit,'f',5));
    set.setValue("axis8Limit", QString::number(axis8Limit,'f',5));
    set.setValue("axis9Limit", QString::number(axis9Limit,'f',5));
    set.setValue("axis10Limit", QString::number(axis10Limit,'f',5));
    set.endGroup();


    set.beginGroup("Offset");
    set.setValue("fodOffset", QString::number(fod,'f',5));
    set.setValue("fddOffset", QString::number(fdd,'f',5));
    set.endGroup();

    set.beginGroup("StepParams");
    set.setValue("startAxis0", QString::number(startAxis0,'f',5));
    set.setValue("startAxis8", QString::number(startAxis8,'f',5));

    set.endGroup();

    set.beginGroup("AxisParams");
    set.setValue("xrayZOffset", QString::number(xrayZOffset, 'f', 5));
    set.setValue("xrayZCoefficient", QString::number(xrayZCoefficient, 'f', 5));
    set.setValue("detectorZOffset", QString::number(detectorZOffset, 'f', 5));
    set.setValue("detectorZCoefficient", QString::number(detectorZCoefficient, 'f', 5));
    set.setValue("detectorROffset", QString::number(detectorROffset, 'f', 5));
    set.setValue("detectorRCoefficient", QString::number(detectorRCoefficient, 'f', 5));
    set.setValue("detectorThetaOffset", QString::number(detectorThetaOffset, 'f', 5));
    set.setValue("detectorThetaCoefficient", QString::number(detectorThetaCoefficient, 'f', 5));
    set.setValue("tableXOffset", QString::number(tableXOffset, 'f', 5));
    set.setValue("tableXCoefficient", QString::number(tableXCoefficient, 'f', 5));
    set.setValue("tableYOffset", QString::number(tableYOffset, 'f', 5));
    set.setValue("tableYCoefficient", QString::number(tableYCoefficient, 'f', 5));
    set.endGroup();

}

void AppConfig::setAxisPos(int value,float pos)
{
    switch (value) {
    case 0:
        axis1 = pos;
        break;
    case 1:
        axis2 = pos;
        break;
    case 2:
        axis3 = pos;
        break;
    case 3:
        axis4 = pos;
        break;
    case 4:
        axis5 = pos;
        break;
    case 5:
        axis6 = pos;
        break;
    case 6:
        axis7 = pos;
        break;
    case 7:
        axis8 = pos;
        break;
    case 8:
        axis9 = pos;
        break;
    case 9:
        axis10 = pos;
        break;
    default:
        break;
    }
    writeConfig();
}

float AppConfig::getAxisPos(int value)
{
    readConfig();
    switch (value) {
    case 0:
        return axis1 ;
        break;
    case 1:
        return axis2 ;
        break;
    case 2:
        return axis3 ;
        break;
    case 3:
        return axis4 ;
        break;
    case 4:
        return axis5 ;
        break;
    case 5:
        return axis6 ;
        break;
    case 6:
        return axis7 ;
        break;
    case 7:
        return axis8 ;
        break;
    case 8:
        return axis9 ;
        break;
    case 9:
        return axis10 ;
        break;
    default:
        break;
    }
}

void AppConfig::setAxisLimitPos(int value, float pos)
{
    switch (value) {
    case 0:
        axis1Limit = pos;
        break;
    case 1:
        axis2Limit = pos;
        break;
    case 2:
        axis3Limit = pos;
        break;
    case 3:
        axis4Limit = pos;
        break;
    case 4:
        axis5Limit = pos;
        break;
    case 5:
        axis6Limit = pos;
        break;
    case 6:
        axis7Limit = pos;
        break;
    case 7:
        axis8Limit = pos;
        break;
    case 8:
        axis9Limit = pos;
        break;
    case 9:
        axis10Limit = pos;
        break;
    default:
        break;
    }
    writeConfig();
}

float AppConfig::getAxisLimitPos(int value)
{
    readConfig();
    switch (value) {
    case 0:
        return axis1Limit ;
        break;
    case 1:
        return axis2Limit ;
        break;
    case 2:
        return axis3Limit ;
        break;
    case 3:
        return axis4Limit ;
        break;
    case 4:
        return axis5Limit ;
        break;
    case 5:
        return axis6Limit ;
        break;
    case 6:
        return axis7Limit ;
        break;
    case 7:
        return axis8Limit ;
        break;
    case 8:
        return axis9Limit ;
        break;
    case 9:
        return axis10Limit ;
        break;
    default:
        break;
    }
}

float AppConfig::getFodOffset()
{
    return fod;
}

float AppConfig::getFddOffset()
{
    return fdd;
}

QString AppConfig::getNDTWorkDir()
{
    readConfig();
    return NDTWorkdir;
}

float AppConfig::getStartAxis0()
{
    readConfig();
    return startAxis0;
}

float AppConfig::getStartAxis8()
{
    readConfig();
    return startAxis8;
}

float AppConfig::getStartAxis_R()
{
    readConfig();
    return startAxis7;
}

float AppConfig::getStartAxis_Y()
{
    readConfig();
    return startAxis2;
}

