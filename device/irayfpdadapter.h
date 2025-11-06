#ifndef IRAYFPDADAPTER_H
#define IRAYFPDADAPTER_H

#include <QObject>
#include <QImage>
// #include "Modules/Detector.h"
#include "detectorcommand.h"
// #include "Modules/EventReceiver.h"
#include "ieventreceiver.h"

struct ImageInfo {
    QImage::Format format = QImage::Format_Invalid;
    int width;
    int height;
    int bytesPerPixel;
    ushort* data = NULL;


    void setCapacity(int size) {
        if (data == NULL) {
            data = new ushort[size];
        }
    }
    ImageInfo() {

    }

    ImageInfo(int w, int h, int b, QImage::Format f) {
        format = f;
        width = w;
        height = h;
        bytesPerPixel = b;
        setCapacity(width * height * bytesPerPixel);
    }
    ~ImageInfo(){
        if (data != NULL) {
            delete data;
        }
    }
};

enum IRayFPDAdapterFPDType {
    IRayFPDAdapterFPDTypeNDT = 0,
    IRayFPDAdapterFPDTypeDTDI
};

enum IRayFPDAdapterMode {
    IRayFPDAdapterModeNormal = 0,
    IRayFPDAdapterModeOffsetTemplate,
    IRayFPDAdapterModeGainDefectTemplate
};

class IRayFpdAdapter : public QObject, iEventReceiver
{
    Q_OBJECT
public:
    ~IRayFpdAdapter();
    //创建对象
    static IRayFpdAdapter* create(const QString& workDirPath, QObject *parent = nullptr);
    //获得检测状态
    enum DetectState
    {
        DS_UNKNOW = 0,
        DS_READY,
        DS_BUSY,
        DS_SLEEPING
    };
    DetectState getDetectState();
    //获得错误信息
    QString getErrorInfo(int error);
    //连接设备
    FPDRESULT connect();
    //断开设备
    FPDRESULT disconnect();
    // 清除缓存
    FPDRESULT cleanCache();
    //准备获取(清除缓存)
    FPDRESULT prepAcquire();
    //单次获取
    FPDRESULT singleAcquire();
    //开始获取
    FPDRESULT startAcquire();
    //停止获取
    FPDRESULT stopAcquire();
    //设置校准模式(偏移量模式 | 增益模式 | 减益模式)
    //偏移量可选模式1.Enm_CorrectOp_SW_PreOffset,2.Enm_CorrectOp_SW_PostOffset,3.Enm_CorrectOp_HW_PreOffset,4.Enm_CorrectOp_HW_PostOffset
    //增益可选模式1.Enm_CorrectOp_SW_Gain,2.Enm_CorrectOp_HW_Gain
    //减益可选模式1.Enm_CorrectOp_SW_Defect,2.Enm_CorrectOp_HW_Defect
    FPDRESULT setCorrectionOption(int correctionOption);
    //读取温度(回调获取值)
    FPDRESULT readTemperature();


    //获得产品类型
    Enm_ProdType getProdType();
    //获得同步模式(产品类型为Enm_Prd_Mercu0909F的触发模式)
    Enm_FluroSync getSyncMode();
    //获得触发模式(其它类型触发模式)
    Enm_TriggerMode getTriggerMode();

    //动态检测设置
    //设置应用模式
    FPDRESULT setApplicationMode(const QString& mode = "Model");
    //设置同步模式
    FPDRESULT setSyncMode(Enm_FluroSync syncMode);

    //静态检测模式
    //设置触发模式
    FPDRESULT setTriggerMode(Enm_TriggerMode triggerMode);

    //其它接口
    FPDRESULT setAttr(int attrId, int value);
    FPDRESULT setAttr(int attrId, float value);
    FPDRESULT setAttr(int attrId, const QString& value);
    void getAttr(int attrId, AttrResult& result);
    FPDRESULT invoke(int cmdId);
    FPDRESULT invoke(int cmdId, int value);
    FPDRESULT invoke(int cmdId, int par1, int par2);
    FPDRESULT invoke(int cmdId, int par1, int par2, int par3);
    FPDRESULT invoke(int cmdId, float value);
    FPDRESULT invoke(int cmdId, const QString& value);
    FPDRESULT setCaliSubset(const QString& subDir);


    // template interfaces

//    FPDRESULT startTemplateAcquiring();
//    FPDRESULT stopTemplateAcquiring();


    FPDRESULT startLightAcqusition();
    FPDRESULT startSelecting4ValidImages();
//    FPDRESULT startSelectingValidFrames();
    FPDRESULT startDarkAcqusition();
    FPDRESULT nextStep();
    FPDRESULT generateTemplateFiles();

    FPDRESULT startMakingNDTOffsetTemplate();
    FPDRESULT startMakingDTDIOffsetTemplate();

    FPDRESULT startMakingNDTGainDefectTemplate();
    FPDRESULT startMakingDTDIGainDefectTemplate();
    FPDRESULT stopMakingGainDefectTemplate();

//    FPDRESULT finishGeneration();
signals:
    //回调事件
    void sigEvent(int eventId, int eventLevel, QString msg,
                  int param1, int param2, int ptrParamLen, void* pParam);
    //已处理回调
    void sig_didConnect(FPDRESULT result);
    void sig_didDisconnect(FPDRESULT result);
    void sig_didStartAcquiring(FPDRESULT resutl);
    void sig_didStopAcquiring(FPDRESULT result);
    void sig_didSetCaliSubset(FPDRESULT result);
    void sig_didWriteUserROM(FPDRESULT result);
    void sig_didChangeParamsOfCurrentAppMode(FPDRESULT result);
    void sig_didSetCorrectOption(FPDRESULT result);

    void sig_didReadTemperature(FPDRESULT result, float temperature);
    void sig_didDefectSelectAll(FPDRESULT result);
    void sig_didGetImage(const QImage& img);
    void sig_didReceiveImageInfo(ImageInfo imgInfo);
    void sig_didReceiveIRayImage(IRayImage *irayImg);

    // template sigs
    void sig_didOffsetGeneration(FPDRESULT result);
    void sig_didGainInit(FPDRESULT resutl);
    void sig_didDefectInit(FPDRESULT resutl);
    void sig_didStartLightAcqusition(FPDRESULT resutl);
    void sig_didSelect4ValidImages(FPDRESULT resutl);
    void sig_didStartDarkAcqusition(FPDRESULT resutl);
    void sig_didGenerateTemplateFiles(FPDRESULT resutl);
    void sig_didFinishGeneration(FPDRESULT resutl);
    void sig_didFinishTemplateAcquiring(FPDRESULT resutl);
    void sig_didFinishMakingTemplate(FPDRESULT result);
    void sig_didGetLightImages(int state);
    void sig_didGetGroupImage(int currGroupIdx, int totalGroupCount);

private:
    explicit IRayFpdAdapter(QObject *parent = nullptr);
    virtual void SDKHandler(int nDetectorID, int nEventID, int nEventLevel, const char* pszMsg,
        int nParam1, int nParam2, int nPtrParamLen, void* pParam) override;
    QImage gray16ImgFromIRayImage(IRayImage *iRayImg);


    FPDRESULT gainInit();
    FPDRESULT defectInit();
    FPDRESULT select4ValidImages(int groupindex, int eachGroupFrames);


    FPDRESULT finishGeneration();
    FPDRESULT getValidFrames();


private:
    CDetector* m_pDetector = nullptr;
    IRayFPDAdapterMode m_mode = IRayFPDAdapterModeNormal;

    HEVENT m_hNextStep = NULL;
    HEVENT m_hErrorEvent = NULL;
    int m_nTotalGroup = 3;
    int m_nEachGroupFrames = 0;
    int m_nCurGroupIndex = 0;
    int m_nStartFrames = 0;
    int m_ExpectedGrays[3] = { 2000, 12000, 4000 };
//    IRayTimer s_timer;

    IRayFPDAdapterFPDType m_fpdType = IRayFPDAdapterFPDTypeNDT;

    int m_gainDefectImgNum = 0;
    int m_acqMode = 0;

    qint64 m_startTimeStamp = 0;
    qint64 m_imgIndex = 0;
    qint64 m_timeCost = 0;
};

#endif // IRayFpdAdapter_H
