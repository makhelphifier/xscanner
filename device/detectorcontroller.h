#ifndef DETECTORVIEWMODEL_H
#define DETECTORVIEWMODEL_H

#include <QObject>
#include <QQueue>
#include <QtConcurrent/QtConcurrent>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include "drivercontroller.h"
// #include "../core/GpuImageRotator.h"

enum DetectorCommandType {
    DETECTOR_CMD_TYPE_NONE = 0,
    DETECTOR_CMD_TYPE_INIT,
    DETECTOR_CMD_TYPE_START_CONNECTING,
    DETECTOR_CMD_TYPE_STOP_CONNECTING,
    DETECTOR_CMD_TYPE_START_ACQUIRING,
    DETECTOR_CMD_TYPE_STOP_ACQUIRING,
    DETECTOR_CMD_TYPE_SET_MODE,
    DETECTOR_CMD_TYPE_SET_FRAME_RATE,
    DETECTOR_CMD_TYPE_START_MULTI_ACQUIRING,
};

struct DetectorCommand {
    DetectorCommandType type = DETECTOR_CMD_TYPE_NONE;
    QVariant var;
};

struct DetectorSavingFileCommand {
    QImage img;
    int frameNumber;
    QString folderPath;
    QString filePath;
};

struct DetectorImageCommand {
    QImage img;
};

struct PixelModeInfo {
    QString mode;
    int Binning;
    int PGA;
    int frequency;
};

enum ImageTaskType {
    TASK_SAVE_PROJECTION,
    TASK_PROCESS_GRAYSCALE16,
    TASK_PROCESS_MULTIFRAME
};

struct ImageTask {
    ImageTaskType type;
    QImage image;
    QRect roi;
    int frameNumber;
    QString folderPath;

    ImageTask() : type(TASK_SAVE_PROJECTION), frameNumber(0) {}
};


enum SyncMode {
    SYNC_MODE_IN = 1,  // 外触发
    SYNC_MODE_OUT = 2  // 内触发
};

// Register meta types for signal/slot system
Q_DECLARE_METATYPE(PixelModeInfo)
Q_DECLARE_METATYPE(QVector<PixelModeInfo>)

// Forward declaration for async save function
void SaveProjectionAsync(const QImage& img, int frameNumber, const QString& folderPath);


class DetectorController : public QObject
{
    Q_OBJECT
    friend class ImageWorkerThread;  // Allow ImageWorkerThread to access private members
public:
    // Singleton instance access
    static DetectorController* getInstance();
    static void destroyInstance();

    // Disable copy constructor and assignment operator
    DetectorController(const DetectorController&) = delete;
    DetectorController& operator=(const DetectorController&) = delete;

    ~DetectorController();

    void initWithWorkDir(const QString& workDir);
    // void setSyncMode(SyncMode mode);
    void startAcquire();
    void stopAcquire();
    void setMode(const QString& mode);
    void setFrameRate(int frameRate);
    int getFrameRate();
    void setWindowLevelRoi(const QRect& roi);
    void setSaveProjection(bool saveProjection, const QString& projectionFolderPath, const QString& saveFilePath="", int saveImageCount = 0);
    void startGetMultiFrames(int framesCount, int m_overlayNumber, bool staticAcquiring=false);

    void setCollectingImage(bool collecting);


    // Public methods for ImageWorkerThread access
    QImage downsampleImagePublic(const QImage& image, int factor = 2);
    QPair<int, int> calculateOptimalWindowLevelPublic(const QImage& image, const QRect& rect);
    QImage applyWindowLevelPublic(const QImage& image, int windowCenter, int windowWidth);
    QImage autoAdjust(const QImage& image, int autoThreshold, int& ww, int& wl);
    QImage autoAdjust16(const QImage& image, int autoThreshold, int& ww, int& wl);


signals:
    void sig_detectorStateChanged(const QString& msg);
    void sig_didConnect(int state);
    void sig_didStartAcquiring(int state);
    void sig_didStopAcquiring(int state);
    void sig_didGetModeList(const QVector<PixelModeInfo>& pixelModeVec);
    void sig_didGetPreviewFramePixmap(const QPixmap& pixmap);
    void sig_didGetMultiFrameImage(const QImage& img, int frameNumber);
    // void sig_didGetOverlayedImage(const QImage& img, int framesIndex);

    void sig_didStartCreateOffsetTemplate();
    void sig_didFinishCreateOffsetTemplate();
private:
    // Private constructor for singleton
    explicit DetectorController(QObject *parent = nullptr);

    void enqueueCommand(DetectorCommand *cmd);
    void processGrayScale16Image(const QImage& originalImage);
    QImage downsampleGrayscale16Simple(const QImage& image, int factor = 2);
    QPair<int, int> calculateOptimalWindowLevel(const QImage& image, const QRect& rect);
    QImage applyWindowLevel(const QImage& image, int windowCenter, int windowWidth);
    QImage adjustWindowLevel16(const QImage &oriImg, int windowCenter, int windowWidth);

    void didReceiveImage(const QImage& img, int frameNumber);
private:
    // Singleton instance and mutex
    static DetectorController* s_instance;
    static QMutex s_mutex;

    QFuture<void> m_overlayFuture;
    QQueue<DetectorImageCommand*> m_overlayImageQueue;
    bool m_overlayLoop = true;

    QFuture<void> m_savingImageFuture;
    QQueue<DetectorSavingFileCommand*> m_savingImageQueue;
    bool m_savingImageLoop = true;

    QFuture<void> m_previewImageFuture;
    QQueue<DetectorImageCommand*> m_previewImageQueue;
    bool m_previewImageLoop = true;


    bool m_saveProjection = true;
    QString m_projectionFolderPath;
    int m_overlayNumber = 1;
    int m_overlayCount = 0;
    int m_didGetOverlayFramesCount = 0;
    int m_framesCount = 0;
    int m_didGetFramesCount = 0;
    // QVector<qint64> m_overlayPixVec;
    QRect m_windowLevelRoi;
    int m_windowCenter = 32768;  // Default window center for 16-bit
    int m_windowWidth = 65536;   // Default window width for 16-bit
    bool m_windowLevelCalculated = false;  // Flag to track if window/level has been calculated
    bool m_staticAcquiring = false;

    int m_frameNumber = 0;
    SyncMode m_syncMode = SYNC_MODE_IN;
    QString m_saveFilePath;
    DriverController *m_driverController = nullptr;

    bool m_collectingImage = false;
    // GpuImageRotator *m_rotator;

    int m_currentSavedImageCount = 0;
    int m_saveImageCount = 0;
    int m_tmpImageCount = 0;
    bool m_rotating = true;  // 控制是否旋转图像，默认为true
};

#endif // DETECTORVIEWMODEL_H
