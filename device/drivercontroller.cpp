#include "drivercontroller.h"
#include <QDir>
#include <QImage>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QPainter>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QApplication>
#include <QSettings>

#define MAXPIXEL 10000

DriverController::DriverController(QObject *parent)
    : QObject{parent}
{
    m_pgaList = QList<Enm_PGA>({Enm_PGA_0, Enm_PGA_1, Enm_PGA_2, Enm_PGA_3, Enm_PGA_4, Enm_PGA_5,
                           Enm_PGA_6, Enm_PGA_7, Enm_PGA_8});
    for (int i = 0; i < m_pgaList.count(); i++) {
        PGAItem item;
        item.text = QString("PGA_%1").arg(i);
        item.value = m_pgaList.at(i);
        m_pgaItemList.append(item);
    }

    for (int i = 0; i < 4; i++) {
        FrequencyItem item;
        item.text = QString::number((i+1)*5);
        item.value = i;
        m_frequencyItemList.append(item);
    }


    QList<Enm_CorrectOption> list;
    list<<Enm_CorrectOp_HW_PreOffset;
    list<<Enm_CorrectOp_HW_PostOffset;
    list<<Enm_CorrectOp_HW_Gain;
    list<<Enm_CorrectOp_HW_Defect;
    list<<Enm_CorrectOp_HW_Ghost;
    list<<Enm_CorrectOp_HW_Lag;
    list<<Enm_CorrectOp_HW_MicroPhony;
    list<<Enm_CorrectOp_HW_GridRemoval;
    list<<Enm_CorrectOp_HW_VirtualGrid;
    list<<Enm_CorrectOp_SW_PreOffset;
    list<<Enm_CorrectOp_SW_PostOffset;
    list<<Enm_CorrectOp_SW_Gain;
    list<<Enm_CorrectOp_SW_Defect;
    list<<Enm_CorrectOp_SW_Ghost;
    list<<Enm_CorrectOp_SW_Lag;
    list<<Enm_CorrectOp_SW_MicroPhony;
    list<<Enm_CorrectOp_SW_GridRemoval;
    list<<Enm_CorrectOp_SW_VirtualGrid;

    m_corrOptHash.insert("Enm_CorrectOp_HW_PreOffset", Enm_CorrectOp_HW_PreOffset);
    m_corrOptHash.insert("Enm_CorrectOp_HW_PostOffset", Enm_CorrectOp_HW_PostOffset);
    m_corrOptHash.insert("Enm_CorrectOp_HW_Gain", Enm_CorrectOp_HW_Gain);
    m_corrOptHash.insert("Enm_CorrectOp_HW_Defect", Enm_CorrectOp_HW_Defect);
    m_corrOptHash.insert("Enm_CorrectOp_HW_Ghost", Enm_CorrectOp_HW_Ghost);
    m_corrOptHash.insert("Enm_CorrectOp_HW_Lag", Enm_CorrectOp_HW_Lag);
    m_corrOptHash.insert("Enm_CorrectOp_HW_MicroPhony", Enm_CorrectOp_HW_MicroPhony);
    m_corrOptHash.insert("Enm_CorrectOp_HW_GridRemoval", Enm_CorrectOp_HW_GridRemoval);
    m_corrOptHash.insert("Enm_CorrectOp_HW_VirtualGrid", Enm_CorrectOp_HW_VirtualGrid);

    m_corrOptHash.insert("Enm_CorrectOp_SW_PreOffset", Enm_CorrectOp_SW_PreOffset);
    m_corrOptHash.insert("Enm_CorrectOp_SW_PostOffset", Enm_CorrectOp_SW_PostOffset);
    m_corrOptHash.insert("Enm_CorrectOp_SW_Gain", Enm_CorrectOp_SW_Gain);
    m_corrOptHash.insert("Enm_CorrectOp_SW_Defect", Enm_CorrectOp_SW_Defect);
    m_corrOptHash.insert("Enm_CorrectOp_SW_Ghost", Enm_CorrectOp_SW_Ghost);
    m_corrOptHash.insert("Enm_CorrectOp_SW_Lag", Enm_CorrectOp_SW_Lag);
    m_corrOptHash.insert("Enm_CorrectOp_SW_MicroPhony", Enm_CorrectOp_SW_MicroPhony);
    m_corrOptHash.insert("Enm_CorrectOp_SW_GridRemoval", Enm_CorrectOp_SW_GridRemoval);
    m_corrOptHash.insert("Enm_CorrectOp_SW_VirtualGrid", Enm_CorrectOp_SW_VirtualGrid);


    QJsonArray jsonArr = readCorrectOptionsFromFile();

    for (int i = 0; i <jsonArr.count(); i++) {
        QString key = jsonArr.at(i).toString();
        if (key != "Enm_CorrectOp_SW_PreOffset" && key != "Enm_CorrectOp_SW_Gain" && key != "Enm_CorrectOp_SW_Defect")
            continue;
        if (m_corrOptHash.contains(key)) {
            m_corrOptList << m_corrOptHash.value(key);
        }
    }
    qDebug()<<"m_corrOptList: "<<m_corrOptList;

}

DriverController::~DriverController()
{
    stopConnecting();

    if (m_pImgBmp != NULL)
        delete[] m_pImgBmp;

    if (m_pMultiImg != NULL)
        delete[] m_pMultiImg;

    if (m_pDetector) {
        delete m_pDetector;
    }
}

/*
void DriverViewModel::startParsingAppModeFile(const QString& workDir)
{
    if (m_parsedAppModeFile)
        return;
    m_parsedAppModeFile = true;

    setWorkDir(workDir);
    emit sig_didSetPGAList(m_pgaItemList);
    emit sig_didSetFrequencyList(m_frequencyItemList);

    QString modeFilePath = workDir+"/DynamicApplicationMode.ini";
    QFile file(modeFilePath);
    QString modeTitle("[ApplicationMode");
    QString modeName = "";
    QString subset = "";
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QHash<QString, QString> params;
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();

            if (line.contains(modeTitle)) {
                if (modeName.compare(line) != 0) {
                    if (!subset.isEmpty() && params.count() > 0) {
                        m_modeMap.insert(subset, params);
                    }
                    params = QHash<QString, QString>();
                }
                modeName = line;
            }
            if (line.contains("=") && !modeName.isEmpty()) {
                QStringList kv = line.split("=");
                QString key = kv.at(0);
                QString val = kv.at(1);

                params.insert(key, val);
                if (key.contains("subset")) {

                    subset = val;
                }
            }
        }
        if (!subset.isEmpty() && params.count() > 0) {
            m_modeMap.insert(subset, params);
        }

        if(params.contains("ROIRange"))
        {
            QString str = "ROIRange_"+params["ROIRange"];
            QHash<QString, QString> params_;
            params_.insert("ROIRange", params["ROIRange"]);
            emit sig_getROIRange(params_);
        }

        file.close();
    }

    m_modeItemList.clear();
    QStringList keys = m_modeMap.keys();
    for (int i = 0; i < keys.count(); i++) {
        AppModeItem item;
        item.text = keys.at(i);
        item.value = i;
        m_modeItemList.append(item);
    }


    // emit sig_didSetModeList(m_modeItemList);

    if (m_modeItemList.count() > 0) {
        setMode(keys.first());
        loadApplicationModeIni();
    }

    qDebug()<<"did load mode sub title: "<<m_modeMap.keys();

}
*/

void DriverController::startParsingAppModeFile2(const QString& workDir)
{
    if (m_parsedAppModeFile)
        return;
    m_parsedAppModeFile = true;

    QString modeFilePath = workDir+"/DynamicApplicationMode.ini";

    // 使用 QSettings 解析 INI 文件
    QSettings settings(modeFilePath, QSettings::IniFormat);

    // 清空现有的模式列表
    m_modeItemList.clear();

    // 获取所有组（group）
    QStringList groups = settings.childGroups();

    // QStringList modeNameList;
    for (int i = 0; i < groups.size(); ++i) {
        const QString& groupName = groups.at(i);

        // 只处理 ApplicationMode 开头的组
        if (!groupName.startsWith("ApplicationMode")) {
            continue;
        }

        settings.beginGroup(groupName);

        // 创建 AppModeItem
        AppModeItem item;

        // 从 subset 字段获取显示文本，如果没有则使用组名
        item.subset = settings.value("subset", groupName).toString();
        item.text = item.subset;
        item.value = i;

        // 读取其他参数
        item.PGA = settings.value("PGA", 2).toInt();
        item.Binning = settings.value("Binning", 0).toInt();
        item.Frequency = settings.value("Frequency", 10).toInt();
        item.ExposureMode = settings.value("ExposureMode", 129).toInt();

        // 解析 ROIRange，格式为 "(x, y, width, height)"
        QString roiStr = settings.value("ROIRange", "(0, 0, 3072, 3060)").toString();
        // 移除括号并按逗号分割
        roiStr = roiStr.remove('(').remove(')');
        QStringList roiParts = roiStr.split(',');
        if (roiParts.size() == 4) {
            int x = roiParts[0].trimmed().toInt();
            int y = roiParts[1].trimmed().toInt();
            int width = roiParts[2].trimmed().toInt();
            int height = roiParts[3].trimmed().toInt();
            item.ROIRange = QRect(x, y, width, height);
        } else {
            // 默认值
            item.ROIRange = QRect(0, 0, 3072, 3060);
        }

        settings.endGroup();

        // 添加到列表
        m_modeItemList.append(item);

        qDebug() << "Parsed AppModeItem:"
                 << "PGA:" << item.PGA
                 << "Binning:" << item.Binning
                 << "Frequency:" << item.Frequency
                 << "ROIRange:" << item.ROIRange
                 << "ExposureMode:" << item.ExposureMode
                 << "subset:" << item.subset;

        // modeNameList << QString("%1: Binning:%2, PGA:%3, Frequency:%4").arg(item.subset).arg(item.Binning).arg(item.PGA).arg(item.Frequency);
    }


    // 发送信号通知模式列表已更新
    // emit sig_didSetModeList(m_modeItemList);
    emit sig_didGetModeList(m_modeItemList);

    if (m_modeItemList.count() > 0) {
        setAppMode(0);
    }


    qDebug() << "startParsingAppModeFile2 completed, loaded" << m_modeItemList.count() << "modes";
    // emit sig_detectorStateChanged("startParsingAppModeFile2 completed, loaded");
}

void DriverController::createDetector(const QString &workDir, Enm_FluroSync syncMode)
{
    m_syncMode = syncMode;

    if (m_pDetector == nullptr) {
        m_workDir = workDir;

        m_pDetector = IRayFpdAdapter::create(workDir);
        if (m_pDetector != nullptr) {
            connect(m_pDetector, &IRayFpdAdapter::sig_didConnect, this, [this](FPDRESULT result) {
                if(result == Err_OK) {
                    emit sig_didConnect(1, u8"连接成功");
                    m_connected = true;

                    m_taskQueue.enqueue(Attr_UROM_FluroSync_W);
                    m_taskQueue.enqueue(ParseAppModeFile);

                    // 禁止设备动态读取ini文件
                    setDynaOffsetEnable(Enm_Off);


                } else {
                    QString errMsg = m_pDetector->getErrorInfo(result);
                    emit sig_didConnect(-1, errMsg);

                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didDisconnect, this, [this](FPDRESULT result) {
                if(result == Err_OK) {
                    emit sig_didDisconnect(1, u8"关闭连接成功");
                    m_connected = false;
                } else {
                    QString errMsg = m_pDetector->getErrorInfo(result);
                    emit sig_didDisconnect(-1, errMsg);
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didStartAcquiring, this, [this](FPDRESULT result) {
                if(result == Err_OK) {
                    qDebug()<<"didStartAcquiring";
                    m_acquiringState = 1;
                    m_autoStartAcquiring = false;

                    m_settingIntervalTime = false;

                    if (m_settingIntervalTime) {
                        return;
                    }


                    m_settingMode = false;
                    if (m_settingMode) {

                    }
                    emit sig_didStartAcquiring(1, u8"开始采集成功");
                } else {
                    QString errMsg = m_pDetector->getErrorInfo(result);
                    emit sig_didStartAcquiring(2, errMsg);
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didStopAcquiring, this, [this](FPDRESULT result) {
                if(result == Err_OK) {
                    m_acquiringState = 2;
                    // 判断如果正在执行设置扫描方向过程中，执行设置扫描方向功能
                    if (m_settingIntervalTime) {
                        setSequenceIntervalTime(m_intervalTime);
                        return;
                    }
                    if (m_settingMode) {
                        setMode(m_settingModeIndex);
                        return;
                    }
                    emit sig_didStopAcquiring(1, u8"停止采集成功");
                } else {
                    QString errMsg = m_pDetector->getErrorInfo(result);
                    emit sig_didStopAcquiring(2, errMsg);
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didSetCaliSubset, this, [this](FPDRESULT result) {
                if(result == Err_OK) {

                    m_currPGA = getPGA();
                    m_currBinning = getBinning();
                    QHash<QString, QString> _params;
                    _params.insert("Binning", QString::number(m_currBinning));
                    emit sig_getBinning(_params);

                    int intervalTime = getSequenceIntervalTime();

                    if (m_settingMode) {
                        setCorrectOption();

                        m_maxFrameRate = 1000/intervalTime;
                        qDebug()<<"did set mode m_maxFrameRate:"<<m_maxFrameRate;
                        if (m_settingMode == 0) {
                            if (m_maxFrameRate > 30) {
                                m_maxFrameRate = 30;
                            }
                        }
                        else if (m_settingMode == 1) {
                            if (m_maxFrameRate > 60) {
                                m_maxFrameRate = 60;
                            }
                        }
                        emit sig_didFinishSetMode(1, QString(u8"当前像素模式%1x%2, 当前帧率更新为%3帧/秒").arg(m_settingModeIndex+1).arg(m_settingModeIndex+1).arg(m_maxFrameRate));
                        emit sig_didFinishSetTimeInterval(m_maxFrameRate);
                        return;
                    }

                    if (m_taskQueue.isEmpty()) {
                        return;
                    }
                    int taskId = m_taskQueue.dequeue();
                    switch(taskId) {
                    case Attr_UROM_FluroSync_W:
                    {
                        setSyncMode(m_syncMode);
                        break;
                    }
                    case Attr_UROM_PGA_W:
                    {
                        QHash<QString, QString> modeHash = m_modeMap.value(m_modeSubset);
                        if (modeHash.contains("PGA")) {
                            bool isDigi;
                            int index = modeHash.value("PGA").toInt(&isDigi);
                            if (isDigi) {
                                PGAItem item = m_pgaItemList.at(index);
                                emit sig_didSelectPGA(item);
                                setPGA((Enm_PGA)item.value);
                            }
                        } else {
                            PGAItem item = m_pgaItemList.at(0);
                            emit sig_didSelectPGA(item);
                            setPGA((Enm_PGA)item.value);
                        }
                        break;
                    }

                    case Attr_UROM_SequenceIntervalTime_W:
                    {
                        QHash<QString, QString> modeHash = m_modeMap.value(m_modeSubset);
                        if (modeHash.contains("Frequency")) {
                            bool isDigi;
                            int frequency = modeHash.value("Frequency").toInt(&isDigi);
                            if (isDigi) {
                                emit sig_didSelectFrequency(frequency);
                                setFrequency(frequency);
                            }
                        }
                        break;
                    }

                    case Cmd_SetCaliSubset:
                        break;
                    case ParseAppModeFile:
                        startParsingAppModeFile2(m_workDir);
                        break;
                    default:
                        createOffsetTemplate();
                        break;
                    }

                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didWriteUserROM, this, [this](FPDRESULT result) {
                if(result == Err_OK) {
                    if (m_taskQueue.isEmpty()) {
                        return;
                    }
                    int taskId = m_taskQueue.dequeue();
                    switch(taskId) {
                    case Attr_UROM_FluroSync_W:
                        setSyncMode(m_syncMode);
                        break;
                    case Attr_UROM_PGA_W:
                    {
                        QHash<QString, QString> modeHash = m_modeMap.value(m_modeSubset);
                        if (modeHash.contains("PGA")) {
                            bool isDigi;
                            int index = modeHash.value("PGA").toInt(&isDigi);
                            if (isDigi) {
                                PGAItem item = m_pgaItemList.at(index);
                                emit sig_didSelectPGA(item);
                                setPGA((Enm_PGA)item.value);
                            }
                        } else {
                            PGAItem item = m_pgaItemList.at(0);
                            emit sig_didSelectPGA(item);
                            setPGA((Enm_PGA)item.value);
                        }
                        break;
                    }

                    case Attr_UROM_SequenceIntervalTime_W:
                    {
                        QHash<QString, QString> modeHash = m_modeMap.value(m_modeSubset);
                        if (modeHash.contains("Frequency")) {
                            bool isDigi;
                            int frequency = modeHash.value("Frequency").toInt(&isDigi);
                            if (isDigi) {
                                emit sig_didSelectFrequency(frequency);
                                setFrequency(frequency);
                            }
                        }
                        break;
                    }

                    case Cmd_SetCaliSubset:
                        break;
                    case ParseAppModeFile:
                        startParsingAppModeFile2(m_workDir);
                        break;
                    default:
                        break;
                    }

                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didGetImage, this, [this](const QImage& img) {

                switch (m_acqType) {
                case AcqTypeNone:
                {
                    emit sig_didReceivePreviewImage(img);
                    break;
                }
                case AcqTypeSingle:
                {
                    if (m_framesCount < 1)
                        return;
                    if (m_framesCount > 0) {
                        emit sig_didGetSingleFrameImage(img);
                    }
                    break;
                }
                case AcqTypeContinuous:
                {
                    if (m_framesCount < 1)
                        return;

                    m_didGetFrames++;
                    if (m_framesCount > 0) {
                        emit sig_didGetMultiFrameImage(img, m_didGetFrames);
                    }
                    m_framesCount--;

                    if (m_framesCount == 0) {
                        m_multiFrameFlag = false;
                        m_acqType = AcqTypeNone;
                    }
                    break;
                }
                default:
                    break;
                }

            });
            connect(m_pDetector, &IRayFpdAdapter::sigEvent, this,
                    [](int eventId, int eventLevel, QString msg,
                    int param1, int param2, int ptrParamLen, void* pParam) {
                Q_UNUSED(eventId);
                Q_UNUSED(eventLevel);
                Q_UNUSED(msg);
                Q_UNUSED(param1);
                Q_UNUSED(param2);
                Q_UNUSED(ptrParamLen);
                Q_UNUSED(pParam);
            });

            connect(m_pDetector, &IRayFpdAdapter::sig_didChangeParamsOfCurrentAppMode, this, [this](FPDRESULT result){
                qDebug()<<"sig_didChangeParamsOfCurrentAppMode"<<result;
                if(result == Err_OK) {
                    qDebug()<<"current PGA: "<<getPGA();
                    int sIntervalTime = getSequenceIntervalTime();
                    qDebug()<<"current SequenceIntervalTime: "<<sIntervalTime<<"ms";

                    emit sig_didFinishSetTimeInterval(1000/sIntervalTime);

                    m_didSetTimeInterval = 1;

                    m_settedCorrectOpt = false;
                    createOffsetTemplate();
                }
                else {
                    m_didSetTimeInterval = 2;
                    startAcquiring();
                    
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didOffsetGeneration, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didOffsetGeneration(1, u8"生成Offset矫正文件成功");
                    qDebug()<<u8"did offset generation OK!";

                    emit sig_didFinishCreateOffsetTemplate();
                    if (m_settedCorrectOpt)
                        return;
                    m_settedCorrectOpt = true;

                    setCorrectOption();
                }
                else {
                    emit sig_didOffsetGeneration(2, u8"生成Offset矫正文件失败");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didSetCorrectOption, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    qDebug()<<u8"did set correct option OK!";
                    m_isSetCorrectOption = true;
                }
                else {
                    qDebug()<<QString(u8"did set correct option fail %1").arg(m_pDetector->getErrorInfo(result));
                }
                if (m_settingIntervalTime && m_autoStartAcquiring) {
                    startAcquiring();
                }
                if (m_settingMode && m_autoStartAcquiring) {
                    startAcquiring();
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didGainInit, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didGainInit(1, u8"初始化Gain+Defect矫正成功");
                }
                else {
                    emit sig_didGainInit(2, u8"初始化Gain+Defect矫正成功");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didDefectInit, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didDefectInit(1, u8"初始化Gain+Defect矫正成功");
                }
                else {
                    emit sig_didDefectInit(2, u8"初始化Gain+Defect矫正成功");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didStartLightAcqusition, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didStartLightAcqusition(1, u8"开启亮场采集成功");
                }
                else {
                    emit sig_didStartLightAcqusition(2, u8"开启亮场采集失败");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didSelect4ValidImages, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didSelect4ValidImages(1, u8"选择有效亮场图像成功");
                }
                else {
                    emit sig_didSelect4ValidImages(2, u8"选择有效亮场图像失败");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didGetLightImages, this, [this](int state){
                emit sig_didGetLightImages(state);
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didGetGroupImage, this, [this](int currGroupIdx, int totalGroupCount){
                emit sig_didGetGroupImage(currGroupIdx, totalGroupCount);
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didStartDarkAcqusition, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didStartDarkAcqusition(1, u8"开启暗场图像采集成功");
                }
                else {
                    emit sig_didStartDarkAcqusition(2, u8"开启暗场图像采集失败");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didGenerateTemplateFiles, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didGenerateTemplateFiles(1, u8"生成Gain+Defect模板文件成功");
                }
                else {
                    emit sig_didGenerateTemplateFiles(2, u8"生成Gain+Defect模板文件失败");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didFinishGeneration, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didFinishGeneration(1, u8"已完成生成矫正操作");
                }
                else {
                    emit sig_didFinishGeneration(2, u8"无法完成生成矫正操作");
                }
            });
            connect(m_pDetector, &IRayFpdAdapter::sig_didFinishMakingTemplate, this, [this](FPDRESULT result){
                if (result == Err_OK) {
                    emit sig_didFinishMakingTemplate(1, u8"停止矫正制作成功");
                }
                else {
                    emit sig_didFinishMakingTemplate(2, u8"停止矫正制作失败");
                }
            });

            emit sig_didCreateDetector(1, u8"创建探测器成功!");
            emit sig_detectorStateChanged(u8"创建探测器成功!");

            startConnecting();

            // startParsingAppModeFile2(workDir);


            qDebug()<<u8"did create detector success";
            m_pTimer = new QTimer(this);
            connect(m_pTimer, &QTimer::timeout, this, [this](){
                getSequenceIntervalTime();
            });
            m_pTimer->start(1000);

        } else {
            // 创建检测器失败
            emit sig_didCreateDetector(2, u8"创建探测器失败");
            emit sig_detectorStateChanged(u8"创建探测器失败");


        }
    }
}

void DriverController::startConnecting()
{
    m_pDetector->connect();
    emit sig_detectorStateChanged(u8"开始连接探测器");
}

void DriverController::stopConnecting()
{
    if (m_pDetector == nullptr)
        return;
    FPDRESULT ret = m_pDetector->disconnect();
    if ((Err_TaskPending != ret) && (Err_OK != ret)) {
        emit sig_didDisconnect(2, u8"关闭连接失败");
    }
}

void DriverController::uninitialize()
{
    if (m_pDetector != nullptr) {
        m_pDetector->deleteLater();
        m_pDetector = nullptr;

        emit sig_didUninitialize();
    }
}

QString DriverController::workDir()
{
    return m_workDir;
}

void DriverController::setWorkDir(const QString &workDir)
{
    m_workDir = workDir;
}

int DriverController::getWW()
{
    return m_ww;
}

void DriverController::setWW(int ww)
{
    m_ww = ww;
}

int DriverController::getWL()
{
    return m_wl;
}

void DriverController::setWL(int wl)
{
    m_wl = wl;
}

QImage DriverController::windowLevelingImg(const QImage &img, int ww, int wl)
{
    int w = img.width();
    int h = img.height();
    int size = img.bytesPerLine()*h;
    ushort *data = new ushort[size];
    memcpy(data, img.constBits(), size);
    ushort *imgTmp = new ushort[size];
    if (m_pImgBmp != NULL)
        delete[] m_pImgBmp;
    m_pImgBmp = new char[size];
    double rate = 256.0 / (double)ww;

    for (int i = 0; i < w*h; i++) {
        int val = 128.0 + (data[i] - wl)*rate;
        if (val < 0) {
            val = 0;
        } else if (val > 255) {
            val = 255;
        }
        imgTmp[i] = val;
    }

    for(int i = 0; i < h; i++) {
        char* rowPtr = m_pImgBmp + i*w;
        for(int j = 0; j < w; j++) {
            rowPtr[j] = imgTmp[i*w+j];
        }
    }

    QImage indexed8((uchar*)m_pImgBmp, w, h, QImage::Format_Indexed8);

    delete[] data;
    delete[] imgTmp;



    return indexed8;

}


void DriverController::startSingleAcquiring()
{
    // m_imgQueue.clear();
    m_framesCount = 1;
    m_acqType = AcqTypeSingle;
    emit sig_didAcquiringTypeChanged(m_acqType);
}


void DriverController::startMultiAcquiring(int framesCount)
{
    qDebug()<<__FUNCTION__<<"startMultiAcquiring: "<<framesCount;
    // m_imgQueue.clear();
    m_framesCount = framesCount;
    m_acqType = AcqTypeContinuous;
    emit sig_didAcquiringTypeChanged(m_acqType, m_framesCount);

    m_didGetFrames = 0;

    m_timeMSecs = QDateTime::currentMSecsSinceEpoch();
    // qDebug()<<"DriverViewModel::startMultiAcquiring: "<<m_timeMSecs<<"  m_acquiring:"<<m_acquiring;
    if (!m_acquiring) {

        startAcquiring();
    }
}

void DriverController::MultiFrameFlag()
{
    m_multiFrameFlag = true;
}

void DriverController::startAcquiring()
{
    if (m_pDetector == nullptr) {
        return;
    }
    if (m_acquiring) {
        return;
    }
    FPDRESULT ret = m_pDetector->startAcquire();
    if ((Err_TaskPending != ret) && (Err_OK != ret)) {
        emit sig_didStartAcquiring(2, u8"开始采集失败");
    }
    else {
        m_acquiring = true;
    }
}

void DriverController::stopAcquiring()
{
    if (m_pDetector == nullptr)
        return;
    FPDRESULT ret = m_pDetector->stopAcquire();
    if ((Err_TaskPending != ret) && (Err_OK != ret)) {
        emit sig_didStopAcquiring(2, u8"停止检测失败");
    }
    else {
        m_acquiring = false;
    }
}

void DriverController::setMode(const QString &val)
{
    if (m_pDetector == nullptr)
        return;
    if (!m_connected)
        return;

    m_modeSubset = val;

    m_taskQueue.enqueue(0);

    AttrResult attr;
    m_pDetector->getAttr(Attr_State, attr);
    if (attr.nVal != Enm_State_Ready)
        return;

    qDebug()<<"val:"<<val;
    FPDRESULT result = m_pDetector->setApplicationMode(val);

    if ((result != Err_OK) && (result != Err_TaskPending)) {
        qDebug()<<u8"设置应用模式失败 "<<" mode:"<<val<<"  "<<m_pDetector->getErrorInfo(result);
        emit sig_didInvoke(QString(u8"设置应用模式失败 %1").arg(m_pDetector->getErrorInfo(result)));
        return;
    } else {
        qDebug()<<u8"设置应用模式成功 "<<" mode:"<<val<<"  "<<m_pDetector->getErrorInfo(result);
        emit sig_didInvoke(QString(u8"设置应用模式成功 %1").arg(val));
    }



}

void DriverController::setMode(int index)
{
    if (m_connected) {
        m_settingMode = true;
        m_settingModeIndex = index;
        if (m_acquiringState == 1) {

            m_autoStartAcquiring = true;
            stopAcquiring();
            return;
        }
        QString modeName = m_modeMap.keys().at(index);
        // qDebug()<<"modeName："<<modeName;
        setMode(modeName);
    }
}
// 设置灵敏度
void DriverController::setPGA(Enm_PGA val)
{
    if (m_pDetector == nullptr)
        return;
    if (!m_connected)
        return;
    m_pga = val;

    AttrResult attr;
    m_pDetector->getAttr(Attr_State, attr);
    if (attr.nVal != Enm_State_Ready)
        return;
    FPDRESULT result = m_pDetector->setAttr(Attr_UROM_PGA_W, val);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didSetAttr(QString(u8"设置灵敏度失败 %1").arg(m_pDetector->getErrorInfo(result)));
    }
    result = m_pDetector->invoke(Cmd_WriteUserROM);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didInvoke(QString(u8"设置灵敏度失败 %1").arg(m_pDetector->getErrorInfo(result)));
    } else {
        emit sig_didInvoke(QString(u8"设置灵敏度成功 %1").arg(val));
    }


}


int DriverController::getPGA()
{
    if (m_pDetector == nullptr)
        return 0;
    if (!m_connected)
        return 0;


    AttrResult attr;
    m_pDetector->getAttr(Attr_State, attr);
    if (attr.nVal != Enm_State_Ready)
        return 0;
    m_pDetector->getAttr(Attr_UROM_PGA, attr);

    return attr.nVal;
}

int DriverController::getBinning()
{
    if (m_pDetector == nullptr)
        return 0;
    if (!m_connected)
        return 0;

    AttrResult attr;
    m_pDetector->getAttr(Attr_State, attr);
    if (attr.nVal != Enm_State_Ready)
        return 0;
    m_pDetector->getAttr(Attr_UROM_BinningMode, attr);

    return attr.nVal;
}

// 设置帧率
void DriverController::setFrequency(int val)
{
    if (m_pDetector == nullptr)
        return;
    if (!m_connected)
        return;
    if (val <= 0)
        return;
    m_frequency = val;

    AttrResult attr;
    m_pDetector->getAttr(Attr_State, attr);
    if (attr.nVal != Enm_State_Ready)
        return;

    int freq = (val == 0 ? 0 : (1000.f / val));
    FPDRESULT result = m_pDetector->setAttr(Attr_UROM_SequenceIntervalTime_W, freq);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        // qDebug()<<"u8设置帧率成功1";
        emit sig_didSetAttr(QString(u8"设置帧率失败 %1").arg(m_pDetector->getErrorInfo(result)));
    }

    result = m_pDetector->invoke(Cmd_WriteUserROM);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        // qDebug()<<"u8设置帧率失败2";
        emit sig_didInvoke(QString(u8"设置帧率失败 %1").arg(m_pDetector->getErrorInfo(result)));
    } else {
        // qDebug()<<"u8设置帧率成功2";
        emit sig_didInvoke(QString(u8"设置帧率成功 间隔%1毫秒").arg(freq));
    }



}

int DriverController::getFrequency()
{
    int intervalTime = getSequenceIntervalTime();
    // m_maxFrameRate = 1000/intervalTime;
    return intervalTime == 0 ? 0 : 1000/intervalTime;
}

void DriverController::setFramesCount(int count)
{
    m_framesCount = count;
}






void DriverController::createOffsetTemplate()
{
    if (m_pDetector == nullptr)
        return;
    if (!m_connected)
        return;

    // qDebug()<<"开始生成Offset 模板";
    FPDRESULT result = m_pDetector->invoke(Cmd_OffsetGeneration);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didSetAttr(QString(u8"生成Offset模板失败 %1").arg(m_pDetector->getErrorInfo(result)));
    }

    emit sig_didStartCreateOffsetTemplate();
}

void DriverController::setCorrectOption()
{
    int nCorrectOption = (Enm_CorrectOp_SW_PreOffset|Enm_CorrectOp_SW_Gain|Enm_CorrectOp_SW_Defect);
    FPDRESULT result = m_pDetector->invoke(Cmd_SetCorrectOption, nCorrectOption);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didSetCorrectOption(2);
    } else {
        emit sig_didSetCorrectOption(1);
    }
}

bool DriverController::startMakingNDTOffsetTemplate()
{
    if (m_pDetector->startMakingNDTOffsetTemplate() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::startMakingDTDIOffsetTemplate()
{
    if (m_pDetector->startMakingDTDIGainDefectTemplate() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::startLightAcquiring()
{
    if (m_pDetector->startLightAcqusition() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::startSelecting4ValidImages()
{
    if (m_pDetector->startSelecting4ValidImages() != Err_OK) {
        return false;
    }
    return true;
}

//void DriverViewModel::startSelectingValidFrames()
//{
//    m_pDetector->startSelectingValidFrames();
//}

bool DriverController::startDarkAcquiring()
{
    if (m_pDetector->startDarkAcqusition() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::nextStep()
{
    if (m_pDetector->nextStep() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::generateTemplateFiles()
{
    if (m_pDetector->generateTemplateFiles() != Err_OK) {
        return false;
    }
//    m_pDetector->finishGeneration();
    return true;
}

bool DriverController::startMakingNDTGainDefectTemplate()
{
    if (m_pDetector->startMakingNDTGainDefectTemplate() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::startMakingDTDIGainDefectTemplate()
{
    if (m_pDetector->startMakingDTDIGainDefectTemplate() != Err_OK) {
        return false;
    }
    return true;
}

bool DriverController::stopMakingGainDefectTemplate()
{
    if (m_pDetector->stopAcquire() != Err_OK) {
        return false;
    }
    return true;
}

void DriverController::setSequenceIntervalTime(qint64 val)  // 单位微秒
{
//    setFrequency(val);
//    return;


    // qDebug()<<"DriverViewModel::setSequenceIntervalTime: "<<val;
//    val = (1000/val)*1000;

    if (m_pDetector == nullptr)
        return;
    if (!m_connected)
        return;
    if (val <= 0)
        return;
    m_intervalTime = val;


    if (m_acquiringState == 1) {
        m_settingIntervalTime = true;
        m_autoStartAcquiring = true;
        stopAcquiring();
        return;
    }

    AttrResult attr;
    m_pDetector->getAttr(Attr_State, attr);
    if (attr.nVal != Enm_State_Ready) {
        // qDebug()<<"setSequenceIntervalTime: "<<attr.strVal;
        return;
    }


    FPDRESULT result = m_pDetector->invoke(Cmd_ChangeParamsOfCurrentAppMode, m_currPGA, m_currBinning, val);
    // qDebug()<<"Cmd_ChangeParamsOfCurrentAppMode: "<<result;
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        qDebug()<<u8"设置拍摄间隔失败:"<<m_pDetector->getErrorInfo(result);
        emit sig_didSetTimeInterval(1, QString(u8"设置拍摄间隔失败:%1").arg(m_pDetector->getErrorInfo(result)));
    }
    else {
//        m_taskQueue.enqueue(Cmd_OffsetGeneration);
    }
}
//探测器没有连接成功，getAttr获取不到attr 会引发崩溃
int DriverController::getSequenceIntervalTime()
{
    if (m_pDetector == nullptr)
        return 1;
    AttrResult attr;
    m_pDetector->getAttr(Attr_UROM_SequenceIntervalTime, attr);
//    qDebug()<<"current sequence intervalTime"<<attr.nVal;
    return attr.nVal;
}

bool DriverController::isConnected()
{
    return m_connected;
}

void DriverController::setOverlayMode(bool mode)
{
    m_overlayMode = mode;
}

bool DriverController::isAcquiring()
{
    return m_acquiringState == 1;
}

void DriverController::setAppMode(int index)
{
    if (index > -1 && index < m_modeItemList.count()) {
        m_currentMode = m_modeItemList.at(index);
    }
    emit sig_didSetAppMode(m_currentMode);
}

int DriverController::currentModeIndex()
{
    return m_settingModeIndex;
}



void DriverController::setSyncMode(Enm_FluroSync mode)
{
    if (m_pDetector == nullptr)
        return;
    FPDRESULT result = m_pDetector->setSyncMode(mode);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didSetAttr(QString(u8"setSyncMode %1").arg(m_pDetector->getErrorInfo(result)));
    } else {
        emit sig_didSetAttr(QString(u8"setSyncMode %1").arg(mode));
    }
}

void DriverController::setDynaOffsetEnable(Enm_Switch s)
{
    if (m_pDetector == nullptr)
        return;
    FPDRESULT result = m_pDetector->setAttr(Attr_UROM_DynaOffsetEnable_W, s);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didSetAttr(QString(u8"Attr_UROM_DynaOffsetEnable_W %1").arg(m_pDetector->getErrorInfo(result)));
    } else {
        emit sig_didSetAttr(QString(u8"Attr_UROM_DynaOffsetEnable_W %1").arg(s));
    }

    result = m_pDetector->invoke(Cmd_WriteUserROM);
    if ((result != Err_OK) && (result != Err_TaskPending)) {
        emit sig_didInvoke(QString(u8"Attr_UROM_DynaOffsetEnable_W Failed %1").arg(m_pDetector->getErrorInfo(result)));
    } else {
        emit sig_didInvoke(QString(u8"Attr_UROM_DynaOffsetEnable_W Success %1").arg(s));
    }
}






QJsonArray DriverController::readCorrectOptionsFromFile()
{
    QJsonArray jsonArr;
    QString appDirPath = QApplication::applicationDirPath();
    QString jsonPath = appDirPath + "/correct_opt.json";
    qDebug()<<"json path:"<<jsonPath;
    QFile file(jsonPath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString jsonStr = in.readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(),&error);
        jsonArr = jsonDoc.array();
        file.close();
    }

    return jsonArr;
}





