#include "detectorcontroller.h"
#include <QPixmap>
#include <QDebug>
#include <QFile>
#include <QMetaObject>
#include <QMutexLocker>
#include <algorithm>
#include <QJsonDocument>
#include <QJsonObject>

#include <opencv2/opencv.hpp>
// #include <opencv2/cudaarithm.hpp>
// #include <opencv2/cudawarping.hpp>

// 使用gpu版本 opencv 实现16位图像顺时针旋转90度

// QImage RotateImage90ClockwiseGPU(const QImage& sourceImage)
// {
//     if (sourceImage.isNull() || sourceImage.format() != QImage::Format_Grayscale16) {
//         qWarning() << "Error: Input image is null or not in Grayscale16 format.";
//         return QImage(); // 返回一个空图像
//     }

//     // 将 QImage 转换为 cv::Mat
//     cv::Mat matImage = cv::Mat(sourceImage.height(), sourceImage.width(), CV_16UC1, (ushort*)sourceImage.bits());

//     // 将 cv::Mat 上传到 GPU
//     cv::cuda::GpuMat gpuImage;
//     gpuImage.upload(matImage);

//     // 计算旋转矩阵
//     cv::Mat rotationMatrix = cv::getRotationMatrix2D(cv::Point2f(gpuImage.cols / 2.0, gpuImage.rows / 2.0), 90, 1.0);

//     // 创建目标 GpuMat
//     cv::cuda::GpuMat gpuRotatedImage;

//     // 使用 warpAffine 进行旋转
//     cv::cuda::warpAffine(gpuImage, gpuRotatedImage, rotationMatrix, gpuImage.size());

//     // 从 GPU 下载结果到 cv::Mat
//     cv::Mat rotatedImage;
//     gpuRotatedImage.download(rotatedImage);

//     // 将 cv::Mat 转换回 QImage
//     return QImage(rotatedImage.data, rotatedImage.cols, rotatedImage.rows, rotatedImage.step[0], QImage::Format_Grayscale16);
// }

QImage RotateImage90ClockwiseCPU(const QImage& sourceImage) {
    if (sourceImage.isNull() || sourceImage.format() != QImage::Format_Grayscale16) {
        qWarning() << "Error: Input image is null or not in Grayscale16 format.";
        return QImage(); // 返回一个空图像
    }

    // 将 QImage 转换为 cv::Mat
    cv::Mat matImage = cv::Mat(sourceImage.height(), sourceImage.width(), CV_16UC1, (ushort*)sourceImage.bits());

    // 创建一个目标 cv::Mat
    cv::Mat rotatedImage;

    // 使用 cv::rotate 进行顺时针旋转 90 度
    cv::rotate(matImage, rotatedImage, cv::ROTATE_90_CLOCKWISE);

    // 将 cv::Mat 转换回 QImage
    return QImage(rotatedImage.data, rotatedImage.cols, rotatedImage.rows, rotatedImage.step[0], QImage::Format_Grayscale16);
}











cv::Mat QImageToMat(const QImage& qimg) {
    // 确保是16位灰度格式
    QImage img = qimg;
    if (img.format() != QImage::Format_Grayscale16) {
        img = img.convertToFormat(QImage::Format_Grayscale16);
    }

    // 创建OpenCV Mat，16位单通道
    cv::Mat mat(img.height(), img.width(), CV_16UC1);

    // 复制数据
    for (int y = 0; y < img.height(); ++y) {
        const uint16_t* srcLine = reinterpret_cast<const uint16_t*>(img.constScanLine(y));
        uint16_t* dstLine = mat.ptr<uint16_t>(y);
        memcpy(dstLine, srcLine, img.width() * sizeof(uint16_t));
    }

    return mat;
}

QImage MatToQImage(const cv::Mat& mat) {
    // 确保是16位单通道
    if (mat.type() != CV_16UC1) {
        qWarning() << "Mat must be 16-bit single channel";
        return QImage();
    }

    // 创建QImage
    QImage qimg(mat.cols, mat.rows, QImage::Format_Grayscale16);

    // 复制数据
    for (int y = 0; y < mat.rows; ++y) {
        const uint16_t* srcLine = mat.ptr<uint16_t>(y);
        uint16_t* dstLine = reinterpret_cast<uint16_t*>(qimg.scanLine(y));
        memcpy(dstLine, srcLine, mat.cols * sizeof(uint16_t));
    }

    return qimg;
}



QImage RotateClockwise90(const QImage& input) {
    // 检查输入图像格式
    if (input.format() != QImage::Format_Grayscale16) {
        qWarning() << "Input image must be 16-bit grayscale format";
        return QImage();
    }

    if (input.isNull()) {
        qWarning() << "Input image is null";
        return QImage();
    }

    // 转换为OpenCV Mat
    cv::Mat mat = QImageToMat(input);

    // 顺时针旋转90度 (使用transpose + flip)
    cv::Mat rotated;
    cv::transpose(mat, rotated);
    cv::flip(rotated, rotated, 1); // 水平翻转



    // 转换回QImage
    return MatToQImage(rotated);
}






bool SaveProjection(const QImage& img, int frameNumber, const QString& folderPath, const QString& saveFilePath)
{
    QString fileName = QString("%1/%2.raw").arg(folderPath).arg(frameNumber-1, 6, 10, QChar('0'));
    if (!saveFilePath.isEmpty()) {
        fileName = saveFilePath;
    }
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        const uchar* data = img.constBits();
        qint64 dataSize = img.sizeInBytes();
        qint64 written = file.write(reinterpret_cast<const char*>(data), dataSize);
        file.close();
        if (written != dataSize) {
            qDebug() << "Failed to write complete image data to file:" << fileName;
        }
    } else {
        qDebug() << "Failed to open file for writing:" << fileName;
        return false;
    }
    // qDebug()<<"did save file to "<<fileName;

    return true;
}

// Static member definitions
DetectorController* DetectorController::s_instance = nullptr;
QMutex DetectorController::s_mutex;



DetectorController* DetectorController::getInstance()
{
    // Double-checked locking pattern for thread safety
    if (s_instance == nullptr) {
        QMutexLocker locker(&s_mutex);
        if (s_instance == nullptr) {
            s_instance = new DetectorController();
        }
    }
    return s_instance;
}

void DetectorController::destroyInstance()
{
    QMutexLocker locker(&s_mutex);
    if (s_instance != nullptr) {
        delete s_instance;
        s_instance = nullptr;
    }
}

DetectorController::DetectorController(QObject *parent)
    : QObject{parent}
{
    // Register meta types for signal/slot system
    qRegisterMetaType<PixelModeInfo>("PixelModeInfo");
    qRegisterMetaType<QVector<PixelModeInfo>>("QVector<PixelModeInfo>");


    // m_rotator = new GpuImageRotator();

    // m_commandFuture = QtConcurrent::run([this](){
    m_driverController = new DriverController(this);

    connect(m_driverController, &DriverController::sig_detectorStateChanged, [this](const QString& msg){
        qDebug()<<"&DriverViewModel::sig_detectorStateChanged: "<<msg;
        emit sig_detectorStateChanged(msg);
    });
    connect(m_driverController, &DriverController::sig_didConnect, [this](int state, const QString& msg = ""){
        emit sig_didConnect(state);
    });
    connect(m_driverController, &DriverController::sig_didStartAcquiring, [this](int state, const QString& msg = ""){
        emit sig_didStartAcquiring(state);
    });
    connect(m_driverController, &DriverController::sig_didStopAcquiring, [this](int state, const QString& msg = ""){
        emit sig_didStopAcquiring(state);
    });
    connect(m_driverController, &DriverController::sig_didGetModeList, [this](const QList<AppModeItem>& list){
        QVector<PixelModeInfo> pixelModeVec;
        for (int i = 0; i < list.count(); i++) {
            PixelModeInfo info;
            AppModeItem item = list.at(i);
            info.mode = item.subset;
            info.Binning = item.Binning;
            info.PGA = item.PGA;
            info.frequency = item.Frequency;

            pixelModeVec.push_back(info);
        }
        emit sig_didGetModeList(pixelModeVec);
    });
    connect(m_driverController, &DriverController::sig_didStartCreateOffsetTemplate, [this](){
        emit sig_didStartCreateOffsetTemplate();
    });
    connect(m_driverController, &DriverController::sig_didFinishCreateOffsetTemplate, [this](){
        emit sig_didFinishCreateOffsetTemplate();
    });
    connect(m_driverController, &DriverController::sig_didSetTimeInterval, [this](int state, const QString& msg = ""){
        // emit sig_didSetTimeInterval();
    });

    connect(m_driverController, &DriverController::sig_didReceivePreviewImage, [this](const QImage& img){
        // qDebug()<<"receive preview image: "<<++m_tmpImageCount;
        if (m_syncMode == SYNC_MODE_IN) {
            if (m_collectingImage) {
                didReceiveImage(img, ++m_frameNumber);
            }
        }
        else {
            DetectorImageCommand *cmd = new DetectorImageCommand;
            cmd->img = img.copy();
            m_previewImageQueue.enqueue(cmd);
        }

    });
    connect(m_driverController, &DriverController::sig_didGetMultiFrameImage, [this](const QImage& img, int frameNumber){
        qDebug()<<"receive multiframe image from driver view model: "<<img;
        if (m_collectingImage) {
            m_frameNumber = frameNumber;
            didReceiveImage(img, frameNumber);
        }
    });


    m_savingImageFuture = QtConcurrent::run([this](){
        while (m_savingImageLoop) {

            if (!m_savingImageQueue.isEmpty()) {
                DetectorSavingFileCommand *cmd = m_savingImageQueue.dequeue();

                // if (++m_currentSavedImageCount <= m_saveImageCount) {
                SaveProjection(cmd->img, cmd->frameNumber, cmd->folderPath, cmd->filePath);
                // qDebug()<<"1 saving projection: "<<cmd->folderPath;
                // qDebug()<<"2 saving projection: "<<cmd->filePath;
                // }
                delete cmd;
            }

            QThread::msleep(10);
        }
    });


    m_overlayFuture = QtConcurrent::run([this](){
        int tempOverlayCount = 0;
        QVector<qint64> overlayPixVec;
        while (m_overlayLoop) {
            if (!m_overlayImageQueue.isEmpty()) {

                DetectorImageCommand *cmd = m_overlayImageQueue.dequeue();
                QImage img = cmd->img;
                int width = img.width();
                int height = img.height();
                int pixCount = width * height;

                if (tempOverlayCount == 0) {
                    qDebug()<<"tempOverlayCount == 0; QVector<qint64>(pixCount, 0);";
                    overlayPixVec = QVector<qint64>(pixCount, 0);
                }

                const ushort *imgData = (const ushort*)img.constBits();
                for (int i = 0; i < pixCount; i++) {
                    overlayPixVec[i] += imgData[i];
                }
                tempOverlayCount++;
                m_didGetFramesCount++;
                if (tempOverlayCount == m_overlayNumber || m_didGetFramesCount == m_framesCount) {
                    QImage overlayedImg(width, height, QImage::Format_Grayscale16);
                    ushort *overlayedData = (ushort*)overlayedImg.bits();
                    for (int i = 0; i < pixCount; i++) {
                        overlayedData[i] = overlayPixVec[i] / m_overlayNumber;
                    }

                    tempOverlayCount = 0;

                    m_didGetOverlayFramesCount++;

                    if (m_staticAcquiring) {

                    }
                    else {
                        emit sig_didGetMultiFrameImage(overlayedImg, m_didGetOverlayFramesCount);
                    }

                    if (m_saveProjection) {
                        DetectorSavingFileCommand *savingFileCmd = new DetectorSavingFileCommand;
                        savingFileCmd->img = overlayedImg.copy();
                        savingFileCmd->frameNumber = m_didGetOverlayFramesCount;
                        savingFileCmd->folderPath = m_projectionFolderPath;
                        savingFileCmd->filePath = m_saveFilePath;

                        m_savingImageQueue.enqueue(savingFileCmd);

                    }

                    qDebug() << "Overlay progress:" << tempOverlayCount << "/" << m_overlayNumber;
                } else {
                    qDebug() << "Overlay progress:" << tempOverlayCount << "/" << m_overlayNumber;
                }

                delete cmd;

            }


            QThread::msleep(10);

        }
    });



    m_previewImageFuture = QtConcurrent::run([this](){

        while (m_previewImageLoop) {

            if (!m_previewImageQueue.isEmpty()) {

                DetectorImageCommand *cmd = m_previewImageQueue.dequeue();
                processGrayScale16Image(cmd->img);
                delete cmd;
            }

            QThread::msleep(10);
        }
    });

}

DetectorController::~DetectorController()
{
    m_driverController->stopAcquiring();
    m_driverController->stopConnecting();
    m_driverController->deleteLater();


    m_overlayLoop = false;
    m_overlayFuture.cancel();
    m_overlayFuture.waitForFinished();
    if (!m_overlayImageQueue.isEmpty()) {
        qDeleteAll(m_overlayImageQueue);
        m_overlayImageQueue.clear();
    }

    m_savingImageLoop = false;
    m_savingImageFuture.cancel();
    m_savingImageFuture.waitForFinished();
    if (!m_savingImageQueue.isEmpty()) {
        qDeleteAll(m_savingImageQueue);
        m_savingImageQueue.clear();
    }

    m_previewImageLoop = false;
    m_previewImageFuture.cancel();
    m_previewImageFuture.waitForFinished();
    if (!m_previewImageQueue.isEmpty()) {
        qDeleteAll(m_previewImageQueue);
        m_previewImageQueue.clear();
    }

    // delete m_rotator;
}

void DetectorController::initWithWorkDir(const QString &workDir)
{
    // Read sync mode from JSON file
    QString configFilePath = QString("%1/detector_config.json").arg(qApp->applicationDirPath());
    QFile configFile(configFilePath);
    Enm_FluroSync syncModeParam = Enm_FluroSync_SyncOut; // Default value
    
    if (configFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = configFile.readAll();
        QJsonDocument document = QJsonDocument::fromJson(jsonData);
        QJsonObject jsonObj = document.object();
        
        if (jsonObj.contains("syncMode")) {
            int syncMode = jsonObj["syncMode"].toInt();
            if (syncMode == 1) {
                syncModeParam = Enm_FluroSync_SyncIn;
                m_syncMode = SYNC_MODE_IN;
            }
            else if (syncMode == 2) {
                syncModeParam = Enm_FluroSync_SyncOut;
                m_syncMode = SYNC_MODE_OUT;
            }
        }
        
        // 读取旋转配置
        if (jsonObj.contains("rotating")) {
            m_rotating = jsonObj["rotating"].toBool();
        }
        
        configFile.close();
    } else {
        qDebug() << "Failed to open config file:" << configFilePath;
    }
    qDebug()<<"detector syncMode: "<<(syncModeParam == 1 ? "Enm_FluroSync_SyncIn" : "Enm_FluroSync_SyncOut");
    m_driverController->createDetector(workDir, syncModeParam);
}

// void DetectorViewModel::setSyncMode(SyncMode mode)
// {
//     if (mode == SYNC_MODE_OUT) {
//         m_driverViewModel->setSyncMode(Enm_FluroSync_SyncOut);
//     }
//     else {
//         m_driverViewModel->setSyncMode(Enm_FluroSync_SyncIn);
//     }

//     m_syncMode = mode;
// }



void DetectorController::processGrayScale16Image(const QImage& originalImage)
{
    // Step 1: Downsample the image to reduce processing load
    // QImage downsampledImage = downsampleGrayscale16Simple(originalImage, 4); // 2x downsampling
    QImage downsampledImage = downsampleGrayscale16Simple(originalImage, 8); // 2x downsampling

    if (!m_windowLevelCalculated) {
        autoAdjust16(downsampledImage.copy(m_windowLevelRoi), 5000, m_windowCenter, m_windowWidth);
        qDebug() << "Window/Level ROI updated: " << m_windowLevelRoi << ", windowCenter: " << m_windowCenter << ", windowWidth: " << m_windowWidth;
        m_windowLevelCalculated = true;
    }
    // // // Step 4: Apply window/level adjustment to the entire downsampled image
    QImage processedImage = applyWindowLevel(downsampledImage, m_windowCenter, m_windowWidth);

    // // Step 5: Emit the processed image
    emit sig_didGetPreviewFramePixmap(QPixmap::fromImage(processedImage));


}

QImage DetectorController::downsampleGrayscale16Simple(const QImage& image, int factor)
{
    if (image.format() != QImage::Format_Grayscale16 || factor <= 1) {
        return image;
    }

    int newWidth = image.width() / factor;
    int newHeight = image.height() / factor;

    if (newWidth <= 0 || newHeight <= 0) {
        return QImage();
    }

    QImage result(newWidth, newHeight, QImage::Format_Grayscale16);

    for (int y = 0; y < newHeight; ++y) {
        const quint16* srcLine = reinterpret_cast<const quint16*>(
            image.constScanLine(y * factor));
        quint16* dstLine = reinterpret_cast<quint16*>(result.scanLine(y));

        for (int x = 0; x < newWidth; ++x) {
            dstLine[x] = srcLine[x * factor];
        }
    }

    return result;
}

QPair<int, int> DetectorController::calculateOptimalWindowLevel(const QImage& image, const QRect& rect)
{
    if (image.format() != QImage::Format_Grayscale16) {
        qWarning() << "Image format is not Grayscale16, cannot calculate window/level";
        return QPair<int, int>(32768, 65536); // Default values for 16-bit
    }

    // Ensure rect is within image bounds
    QRect validRect = rect.intersected(image.rect());
    if (validRect.isEmpty()) {
        qWarning() << "Invalid rect for window/level calculation";
        return QPair<int, int>(32768, 65536);
    }

    // Collect pixel values from the specified region
    QVector<quint16> pixelValues;
    pixelValues.reserve(validRect.width() * validRect.height());

    for (int y = validRect.top(); y <= validRect.bottom(); ++y) {
        const quint16* scanLine = reinterpret_cast<const quint16*>(image.constScanLine(y));
        for (int x = validRect.left(); x <= validRect.right(); ++x) {
            pixelValues.append(scanLine[x]);
        }
    }

    if (pixelValues.isEmpty()) {
        return QPair<int, int>(32768, 65536);
    }

    // Sort pixel values to calculate percentiles
    std::sort(pixelValues.begin(), pixelValues.end());

    // Calculate statistics
    int minValue = pixelValues.first();
    int maxValue = pixelValues.last();

    // Use 1st and 99th percentiles to exclude outliers
    int p1Index = pixelValues.size() * 0.01;
    int p99Index = pixelValues.size() * 0.99;
    int p1Value = pixelValues[qMax(0, p1Index)];
    int p99Value = pixelValues[qMin(pixelValues.size() - 1, p99Index)];

    // Calculate window center (middle of the range)
    int windowCenter = (p1Value + p99Value) / 2;

    // Calculate window width (range between percentiles)
    int windowWidth = p99Value - p1Value;

    // Ensure minimum window width
    if (windowWidth < 100) {
        windowWidth = 100;
    }

    qDebug() << "Window/Level calculation:"
             << "ROI:" << validRect
             << "Min:" << minValue << "Max:" << maxValue
             << "P1:" << p1Value << "P99:" << p99Value
             << "Window Center:" << windowCenter << "Window Width:" << windowWidth;

    return QPair<int, int>(windowCenter, windowWidth);
}

QImage DetectorController::applyWindowLevel(const QImage& image, int windowCenter, int windowWidth)
{
    if (image.format() != QImage::Format_Grayscale16) {
        qWarning() << "Image format is not Grayscale16, cannot apply window/level"<<image;
        return image;
    }

    // Create output image in 8-bit grayscale format for display
    QImage outputImage(image.width(), image.height(), QImage::Format_Grayscale8);

    // Calculate window bounds
    int windowMin = windowCenter - windowWidth / 2;
    int windowMax = windowCenter + windowWidth / 2;

    // Ensure valid window bounds
    if (windowMax <= windowMin) {
        windowMax = windowMin + 1;
    }

    double scale = 255.0 / (windowMax - windowMin);

    // Apply window/level transformation
    for (int y = 0; y < image.height(); ++y) {
        const quint16* inputLine = reinterpret_cast<const quint16*>(image.constScanLine(y));
        quint8* outputLine = outputImage.scanLine(y);

        for (int x = 0; x < image.width(); ++x) {
            int pixelValue = inputLine[x];

            // Apply window/level transformation
            int outputValue;
            if (pixelValue <= windowMin) {
                outputValue = 0;
            } else if (pixelValue >= windowMax) {
                outputValue = 255;
            } else {
                outputValue = static_cast<int>((pixelValue - windowMin) * scale);
            }

            outputLine[x] = static_cast<quint8>(qBound(0, outputValue, 255));
        }
    }


    return outputImage;
}

QImage DetectorController::adjustWindowLevel16(const QImage &oriImg, int windowCenter, int windowWidth)
{
    int minValue = windowCenter - windowWidth / 2;
    int maxValue = windowCenter + windowWidth / 2;


    int pixelCount = oriImg.width() * oriImg.height();

    // QImage newImg = QImage(oriImg.width(), oriImg.height(), oriImg.format());
    QImage newImg = QImage(oriImg.width(), oriImg.height(), QImage::Format_Grayscale8);
    ushort *oriData = (ushort*)oriImg.constBits();
    uchar *data = (uchar*)newImg.bits();


    for (int i = 0; i < pixelCount; i++) {
        ushort gray = oriData[i];
        if (gray < minValue) {
            gray = 0;
        }
        else if (gray > maxValue) {
            gray = 65535;
        }
        else {
            gray = static_cast<int>((gray - minValue) * 65535.0 / windowWidth);
        }
        data[i] = gray/65535.0*255;
    }



    return newImg;
}

void DetectorController::didReceiveImage(const QImage &img, int frameNumber)
{
    QImage rotatedImage = m_rotating ? RotateClockwise90(img) : img;
    // QImage rotatedImage = img;



    if (m_overlayNumber > 1 && m_overlayCount < m_overlayNumber) {
        DetectorImageCommand *overlayImageCmd = new DetectorImageCommand;
        overlayImageCmd->img = rotatedImage;
        m_overlayImageQueue.enqueue(overlayImageCmd);

        if (m_overlayCount == m_overlayNumber) {
            m_overlayCount = 0;
        }
    }
    else {
        m_didGetFramesCount++;


        if (m_saveProjection && m_didGetFramesCount <= m_saveImageCount) {
            DetectorSavingFileCommand *cmd = new DetectorSavingFileCommand;
            cmd->img = rotatedImage;
            cmd->frameNumber = m_didGetFramesCount;
            cmd->folderPath = m_projectionFolderPath;
            cmd->filePath = m_saveFilePath;

            m_savingImageQueue.enqueue(cmd);
            // qDebug()<<"enqueue image to saving image queue: "<<m_didGetFramesCount<<"/"<<m_saveImageCount;
        }

        if (m_staticAcquiring) {

        }
        else {
            emit sig_didGetMultiFrameImage(rotatedImage, m_didGetFramesCount);
        }


    }

    // DetectorImageCommand *cmd = new DetectorImageCommand;
    // cmd->img = rotatedImage;
    // m_previewImageQueue.enqueue(cmd);
}

// Public wrapper methods for ImageWorkerThread
QImage DetectorController::downsampleImagePublic(const QImage& image, int factor)
{
    return downsampleGrayscale16Simple(image, factor);
}

QPair<int, int> DetectorController::calculateOptimalWindowLevelPublic(const QImage& image, const QRect& rect)
{
    return calculateOptimalWindowLevel(image, rect);
}

QImage DetectorController::applyWindowLevelPublic(const QImage& image, int windowCenter, int windowWidth)
{
    return applyWindowLevel(image, windowCenter, windowWidth);
}

QImage DetectorController::autoAdjust(const QImage& image, int autoThreshold, int& ww, int& wl)
{
    if (image.format() != QImage::Format_Grayscale16) {
        qWarning() << "Image format is not Grayscale16, cannot apply auto adjust";
        return image;
    }

    QImage resultImage = image.copy();
    int width = image.width();
    int height = image.height();
    int pixelCount = width * height;
    int limit = pixelCount / 10;

    // Find min and max values in the image
    const quint16* imageData = reinterpret_cast<const quint16*>(image.constBits());
    quint16 minVal = 65535;
    quint16 maxVal = 0;

    for (int i = 0; i < pixelCount; i++) {
        quint16 pixel = imageData[i];
        if (pixel < minVal) minVal = pixel;
        if (pixel > maxVal) maxVal = pixel;
    }

    // Create histogram with 256 bins
    QVector<int> hist(256, 0);
    double binSize = (maxVal - minVal + 0.0) / 256.0;
    double scale = 256.0 / (maxVal - minVal);

    // Generate histogram
    for (int i = 0; i < pixelCount; i++) {
        quint16 pixel = imageData[i];
        if (pixel >= minVal && pixel <= maxVal) {
            int index = static_cast<int>(scale * (pixel - minVal));
            if (index > 255) index = 255;
            if (index < 0) index = 0;
            hist[index]++;
        }
    }

    int m_autoThreshold = autoThreshold;
    int threshold = pixelCount / m_autoThreshold;

    // Find hmin
    int hmin = 0;
    bool found = false;
    for (int i = 0; i < 256 && !found; i++) {
        int count = hist[i];
        if (count > limit) count = 0;
        found = count > threshold;
        if (!found) hmin = i + 1;
    }

    // Find hmax
    int hmax = 255;
    found = false;
    for (int i = 255; i >= 0 && !found; i--) {
        int count = hist[i];
        if (count > limit) count = 0;
        found = count > threshold;
        if (!found) hmax = i - 1;
    }

    if (hmax >= hmin) {
        double min = minVal + hmin * binSize;
        double max = minVal + hmax * binSize;

        if (min == max) {
            min = minVal;
            max = maxVal;
        }

        ww = static_cast<int>(max - min);
        wl = static_cast<int>((max + min) / 2);

        double scale1 = 65535.0 / (max - min);

        // Apply the transformation to result image
        quint16* resultData = reinterpret_cast<quint16*>(resultImage.bits());
        for (int i = 0; i < pixelCount; i++) {
            double value = imageData[i];
            value = (value - min) * scale1;

            if (value < 0.0) {
                value = 0.0;
            } else if (value > 65535.0) {
                value = 65535.0;
            }

            resultData[i] = static_cast<quint16>(value + 0.5);
        }
    }

    return resultImage;
}

// #include <vector>
// #include <algorithm>
// #include <limits>
// #include <cmath>
QImage DetectorController::autoAdjust16(const QImage& image, int autoThreshold, int& ww, int& wl)
{

    // 创建结果图像
    QImage resultImage = image.copy();
    QImage histImage = resultImage.convertToFormat(QImage::Format_Grayscale16); // 转换为 16 位灰度

    int pixelCount = histImage.width() * histImage.height();
    int limit = pixelCount / 10;


    double minVal = 1e300; // Replace with a sufficiently large constant
    double maxVal = std::numeric_limits<double>::lowest();

    for (int y = 0; y < histImage.height(); ++y) {
        const ushort *line = reinterpret_cast<const ushort *>(histImage.scanLine(y));
        for (int x = 0; x < histImage.width(); ++x) {
            double pixel = line[x];
            if (pixel < minVal) minVal = pixel;
            if (pixel > maxVal) maxVal = pixel;
        }
    }

    // 生成直方图
    std::vector<float> hist(256, 0.0f);
    double binSize = (maxVal - minVal) / 256.0;
    double scale = 256.0 / (maxVal - minVal);

    for (int y = 0; y < histImage.height(); ++y) {
        const ushort *line = reinterpret_cast<const ushort *>(histImage.scanLine(y));
        for (int x = 0; x < histImage.width(); ++x) {
            double pixel = line[x];
            if (pixel >= minVal && pixel <= maxVal) {
                int index = static_cast<int>(scale * (pixel - minVal));
                index = std::clamp(index, 0, 255);
                hist[index]++;
            }
        }
    }

    // 自动阈值计算
    int threshold = pixelCount / autoThreshold;
    int hmin = 0;
    int hmax = 255;

    // 找到 hmin
    for (int i = 0; i < 256; ++i) {
        if (hist[i] > limit) hist[i] = 0.0f;
        if (hist[i] > threshold) {
            hmin = i;
            break;
        }
    }

    // 找到 hmax
    for (int i = 255; i >= 0; --i) {
        if (hist[i] > limit) hist[i] = 0.0f;
        if (hist[i] > threshold) {
            hmax = i;
            break;
        }
    }

    // 调整像素值范围
    if (hmax >= hmin) {
        double min = minVal + hmin * binSize;
        double max = minVal + hmax * binSize;
        if (min == max) {
            min = minVal;
            max = maxVal;
        }

        ww = static_cast<int>(max - min);
        wl = static_cast<int>((max + min) / 2);

        double scale1 = 65535.0 / (max - min);

        for (int y = 0; y < histImage.height(); ++y) {
            ushort *line = reinterpret_cast<ushort *>(resultImage.scanLine(y));
            const ushort *srcLine = reinterpret_cast<const ushort *>(histImage.scanLine(y));
            for (int x = 0; x < histImage.width(); ++x) {
                double value = srcLine[x];
                value = (value - min) * scale1;
                value = std::clamp(value, 0.0, 65535.0);
                line[x] = static_cast<ushort>(value + 0.5);
            }
        }
    }

    return resultImage;
}

// #include "detectorviewmodel.moc"

void DetectorController::startAcquire()
{
    m_driverController->startAcquiring();
}

void DetectorController::stopAcquire()
{
    m_driverController->stopAcquiring();
}

void DetectorController::setMode(const QString &mode)
{
    m_driverController->setMode(mode);
}

void DetectorController::setFrameRate(int frameRate)
{
    m_driverController->setFrequency(frameRate);
}

int DetectorController::getFrameRate()
{
    return m_driverController->getFrequency();
}

void DetectorController::setWindowLevelRoi(const QRect &roi)
{
    m_windowLevelRoi = roi;
    // Reset the flag to force recalculation of window/level on next image
    m_windowLevelCalculated = false;

}

void DetectorController::setSaveProjection(bool saveProjection, const QString& projectionFolderPath, const QString& saveFilePath, int saveImageCount)
{
    m_saveProjection = saveProjection;
    m_projectionFolderPath = projectionFolderPath;
    m_saveFilePath = saveFilePath;
    m_currentSavedImageCount = 0;
    m_saveImageCount = saveImageCount;
}

void DetectorController::startGetMultiFrames(int framesCount, int overlayNumber, bool staticAcquiring)
{
    m_collectingImage = false;
    m_frameNumber = 0;
    m_didGetFramesCount = 0;
    m_overlayCount = 0;
    m_didGetOverlayFramesCount = 0;
    m_overlayNumber = overlayNumber;
    m_framesCount = framesCount;
    m_staticAcquiring = staticAcquiring;
    m_tmpImageCount = 0;

    if (m_syncMode == SYNC_MODE_OUT) {
        m_driverController->startMultiAcquiring(m_framesCount);
    }
}

void DetectorController::setCollectingImage(bool collecting)
{
    m_collectingImage = collecting;
}



