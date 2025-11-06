#ifndef DRIVERCONTROLLER_H
#define DRIVERCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QMap>
#include <QQueue>
#include "irayfpdadapter.h"

#define ParseAppModeFile  100000

using namespace std;

struct PGAItem {
    QString text = "";
    int value = -1;
};

struct AppModeItem {
    QString text;
    int value;
    int PGA;
    int Binning;
    QRect ROIRange;
    int Frequency;
    int ExposureMode;
    QString subset;
};

struct FrequencyItem
{
    QString text = "";
    int value = -1;
    FrequencyItem() {}
};

enum AcqType {
    AcqTypeNone = 0,
    AcqTypeSingle,
    AcqTypeContinuous
};

class IRayFpdAdapter;

class DriverController : public QObject
{
    Q_OBJECT
public:
    explicit DriverController(QObject *parent = nullptr);
    ~DriverController();
    // void startParsingAppModeFile(const QString& workDir);
    void startParsingAppModeFile2(const QString& workDir);
    void createDetector(const QString& workDir, Enm_FluroSync syncMode);    // 1、内触发，2、外触发
    void startConnecting();
    void stopConnecting();
    void uninitialize();
    QString workDir();
    void setWorkDir(const QString& workDir);
    int getWW();
    void setWW(int ww);
    int getWL();
    void setWL(int wl);
    QImage windowLevelingImg(const QImage &img, int ww, int wl);

    void startSingleAcquiring();    // 单帧拍摄
    void startMultiAcquiring(int framesCount);  // 连续拍摄

    void MultiFrameFlag();

    void startAcquiring();
    void stopAcquiring();
    void setMode(const QString& val);
    void setMode(int index);
    void setPGA(Enm_PGA val);
    int getPGA();
    int getBinning();
    void setFrequency(int val);
    int getFrequency();
    void setFramesCount(int count);
    // void saveImage(const QImage& img);
    void saveImage(const QQueue<QImage>& imgQueue);

    void createOffsetTemplate();
    void setCorrectOption();


    // template interface
    bool startMakingNDTOffsetTemplate();
    bool startMakingDTDIOffsetTemplate();

    bool startLightAcquiring();
    bool startSelecting4ValidImages();
    bool startDarkAcquiring();
    bool nextStep();
    bool generateTemplateFiles();

    bool startMakingNDTGainDefectTemplate();
    bool startMakingDTDIGainDefectTemplate();
    bool stopMakingGainDefectTemplate();

    void setSequenceIntervalTime(qint64 val);
    int getSequenceIntervalTime();
    bool isConnected();
    void setOverlayMode(bool mode);
    bool isAcquiring();

    void setAppMode(int index);

    int currentModeIndex();

    int maxFrameRate() { return m_maxFrameRate; }
    bool isSetCorrectOption() { return m_isSetCorrectOption; }

    void setSyncMode(Enm_FluroSync mode);

signals:
    void sig_didCreateDetector(int state, const QString& msg);
    void sig_didUninitialize();
    void sig_didConnect(int state, const QString& msg = "");
    void sig_didDisconnect(int state, const QString& msg = "");
    void sig_didReceivePreviewImage(const QImage& img);
    void sig_didCleanCache(int state, const QString& msg = "");
    void sig_didSingleAcquiring(int state, const QString& msg = "");
    void sig_didStartAcquiring(int state, const QString& msg = "");
    void sig_didStopAcquiring(int state, const QString& msg = "");
    void sig_didSaveImage(const QImage& img);


    void sig_detectorStateChanged(const QString& stateMsg);

    // void sig_didSetModeList(const QList<AppModeItem>& list);
    void sig_didGetModeList(const QList<AppModeItem>& list);
    void sig_didSetPGAList(const QList<PGAItem>& list);
    void sig_didSetFrequencyList(const QList<FrequencyItem>& list);
    void sig_didSelectPGA(const PGAItem& item);
    void sig_didSelectFrequency(int val);
    void sig_didAcquiringTypeChanged(AcqType type, int framesCount = 1);
    void sig_didInvoke(const QString& msg);
    void sig_didSetAttr(const QString& msg);
    void sig_didSetCorrectOption(int state);

    void sig_didOffsetGeneration(int state, const QString& msg = "");
    void sig_didGainInit(int state, const QString& msg = "");
    void sig_didDefectInit(int state, const QString& msg = "");
    void sig_didStartLightAcqusition(int state, const QString& msg = "");
    void sig_didSelect4ValidImages(int state, const QString& msg = "");
    void sig_didStartDarkAcqusition(int state, const QString& msg = "");
    void sig_didGenerateTemplateFiles(int state, const QString& msg = "");
    void sig_didFinishGeneration(int state, const QString& msg = "");
    void sig_didFinishTemplateAcquiring(int state, const QString& msg = "");
    void sig_didFinishMakingTemplate(int state, const QString& msg = "");

    void sig_didGetLightImages(int state);
    void sig_didGetGroupImage(int currGroupIdx, int totalGroupCount);

    void sig_didGetSingleFrameImage(const QImage& img);
    void sig_didGetMultiFrameImage(const QImage& img, int frameNumber);
    void sig_didSetTimeInterval(int state, const QString& msg = "");

    void sig_didSetAppMode(const AppModeItem& appModeItem);
    void sig_didFinishSetTimeInterval(int val);

    void sig_didStartCreateOffsetTemplate();
    void sig_didFinishCreateOffsetTemplate();

    void sig_didFinishSetMode(int state, const QString& text);

    void sig_getROIRange( QHash<QString, QString> val);//获取探测器的区间距离
    void sig_getBinning( QHash<QString, QString> val);//获取探测器的区间距离
private:

    void setDynaOffsetEnable(Enm_Switch s);
    QJsonArray readCorrectOptionsFromFile();
private:
    QString m_workDir = "";
    IRayFpdAdapter *m_pDetector{};
//    QTimer *m_pStateTimer{};
    bool m_connected = false;
    bool m_acquiring = false;
    int m_ww = 1;
    int m_wl = 1;
    Enm_FluroSync m_syncMode = Enm_FluroSync_SyncOut;
    Enm_PGA m_pga = Enm_PGA_0;
    int m_frequency = 1;
    QList<Enm_PGA> m_pgaList;
    QList<PGAItem> m_pgaItemList;
    QList<AppModeItem> m_modeItemList;
    QList<FrequencyItem> m_frequencyItemList; // Frequency

    QMap<QString, QHash<QString, QString>> m_modeMap;
    int m_framesCount = 1;
    // QQueue<QImage> m_imgQueue;
    // int m_syncMode = 1;
    AcqType m_acqType = AcqTypeNone;
    QImage m_rawImg;
    bool m_parsedAppModeFile = false;
    char *m_pImgBmp = NULL;
    ushort *m_pMultiImg = NULL;
    QQueue<int> m_taskQueue;
    QString m_modeSubset = "";

    QHash<QString, Enm_CorrectOption> m_corrOptHash;
    QList<Enm_CorrectOption> m_corrOptList;


    int m_acquiringState = 2;
    bool m_settingIntervalTime = false;
    int m_didSetTimeInterval = 2;
    qint64 m_intervalTime = 0;
    bool m_settedCorrectOpt = false;


    bool m_overlayMode = false;
    bool m_autoStartAcquiring = false;
    AppModeItem m_currentMode;
    // QVector<AppModeItem> m_appModeItemVec;

    bool m_settingMode = false;
    int m_settingModeIndex = 0;
    int m_maxFrameRate = 30;
    int m_currPGA = 0;
    qint64 m_timeMSecs = 0;
    int m_currBinning = 0;
    int m_didGetFrames = 0;
    bool m_multiFrameFlag = false;

    QTimer *m_pTimer = nullptr;
    bool m_isSetCorrectOption = false;
};

#endif // DRIVERCONTROLLER_H
