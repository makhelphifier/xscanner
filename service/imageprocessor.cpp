// service/imageprocessor.cpp

#include "imageprocessor.h"
#include <QVector>
#include <QtMath>
#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <QDataStream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <limits>

ImageProcessor::ImageProcessor() {}

cv::Mat ImageProcessor::readRawImg_cvMat(const QString imgPath, const int width, const int height, int cvType)
{
    cv::Mat image(height, width, cvType);

    QFile file(imgPath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Error opening file: " << imgPath.toStdString() << std::endl;
        return cv::Mat();
    }

    QByteArray data = file.readAll();
    file.close();

    qint64 expectedSize = static_cast<qint64>(width) * height * image.elemSize();
    if (data.size() != expectedSize) {
        std::cerr << "File size mismatch: expected " << expectedSize << ", got " << data.size() << std::endl;
        return cv::Mat();
    }

    memcpy(image.data, data.constData(), data.size());

    return image;
}

QImage ImageProcessor::cvMat2QImage(const cv::Mat& mat)
{
    switch(mat.type())
    {
    // 8位无符号，单通道
    case CV_8UC1:
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return image.copy();
    }
    // 8位无符号，3通道
    case CV_8UC3:
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    // 8位无符号，4通道
    case CV_8UC4:
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    // 16位无符号，单通道
    case CV_16UC1:
    {
        QImage image(reinterpret_cast<const uchar*>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_Grayscale16);
        return image.copy();
    }
    // 32位浮点，单通道（归一化到 8 位灰度）
    case CV_32FC1:
    {
        cv::Mat normalized;
        cv::normalize(mat, normalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        return cvMat2QImage(normalized);
    }
    default:
        qWarning() << "Unsupported cv::Mat type:" << mat.type();
        break;
    }
    return QImage();
}

QImage ImageProcessor::readRawImg_qImage(const QString imgPath, const int width, const int height)
{
    // 创建一个Mat对象，用于存储16位无符号整数数据
    cv::Mat image(height, width, CV_16UC1);

    // 使用 QFile 处理 Qt 资源路径（支持 ":/" 前缀）或普通文件路径
    QFile file(imgPath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Error opening file: " << imgPath.toStdString() << std::endl;
        return QImage();
    }

    // 读取所有数据
    QByteArray data = file.readAll();
    file.close();

    // 检查数据长度是否匹配预期
    qint64 expectedSize = static_cast<qint64>(width) * height * sizeof(uint16_t);
    if (data.size() != expectedSize) {
        std::cerr << "File size mismatch: expected " << expectedSize << ", got " << data.size() << std::endl;
        return QImage();
    }

    // 将 QByteArray 数据复制到 cv::Mat
    memcpy(image.data, data.constData(), data.size());

    // 直接返回16位QImage，而不进行归一化（用于后续窗位窗宽处理）
    return cvMat2QImage(image);
}

cv::Mat ImageProcessor::QImage2cvMat(const QImage &image)
{
    cv::Mat mat;
    switch(image.format())
    {
    case QImage::Format_Grayscale8:
        // QImage 和 cv::Mat 共享数据 (无复制)
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
        break;
    case QImage::Format_Grayscale16:
        // QImage 和 cv::Mat 共享数据 (无复制)
        mat = cv::Mat(image.height(), image.width(), CV_16UC1, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR); // Qt 是 RGB, OpenCV 是 BGR
        break;
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
        break;
    default:
        // 不支持的格式，进行转换
        QImage temp = image.convertToFormat(QImage::Format_Grayscale8);
        mat = cv::Mat(temp.height(), temp.width(), CV_8UC1, const_cast<uchar*>(temp.constBits()), temp.bytesPerLine());
        break;
    }
    // 返回一个克隆，确保数据被复制，防止生命周期问题
    return mat.clone();
}

void ImageProcessor::calculateAutoWindowLevel(const cv::Mat &image, double &min, double &max, double saturatedRatio)
{
    if (image.empty()) {
        min = 0.0;
        max = 255.0;
        return;
    }

    long long pixelCount = image.total();
    int saturatedPixels = qRound(static_cast<double>(pixelCount) * saturatedRatio);

    if (image.type() == CV_32F || image.type() == CV_64F) {
        cv::minMaxLoc(image, &min, &max);
        if (min >= max) {
            min = 0.0;
            max = 1.0;
            return;
        }

        int histSize = 256;
        float range[] = { static_cast<float>(min), static_cast<float>(max) };
        const float* histRange = { range };
        cv::Mat hist;
        cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

        float binWidth = (max - min) / histSize;

        int cumulativeCount = 0;
        for (int i = 0; i < histSize; ++i) {
            cumulativeCount += static_cast<int>(hist.at<float>(i) * pixelCount / hist.total()); // 调整为像素计数
            if (cumulativeCount >= saturatedPixels) {
                min = min + i * binWidth;
                break;
            }
        }

        cumulativeCount = 0;
        for (int i = histSize - 1; i >= 0; --i) {
            cumulativeCount += static_cast<int>(hist.at<float>(i) * pixelCount / hist.total());
            if (cumulativeCount >= saturatedPixels) {
                max = min + i * binWidth;
                break;
            }
        }

    } else {
        int maxVal;
        switch (image.type()) {
        case CV_8U: maxVal = 255; break;
        case CV_16U: maxVal = 65535; break;
        case CV_32S: maxVal = 2147483647; break; // 假设正值范围
        default: maxVal = 255; break;
        }

        // 对于大范围（如 16U），使用 OpenCV 的直方图以提高效率
        int histSize = (maxVal > 65535) ? 65536 : (maxVal + 1); // 限制 histSize 以避免内存问题
        cv::Mat hist;
        int histType = CV_32F;
        float range[] = { 0, static_cast<float>(maxVal + 1) };
        const float* histRanges = { range };
        cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRanges);

        int cumulativeCount = 0;
        min = 0.0;
        for (int i = 0; i < histSize; ++i) {
            cumulativeCount += static_cast<int>(hist.at<float>(i));
            if (cumulativeCount >= saturatedPixels) {
                min = static_cast<double>(i);
                break;
            }
        }

        cumulativeCount = 0;
        max = static_cast<double>(maxVal);
        for (int i = histSize - 1; i >= 0; --i) {
            cumulativeCount += static_cast<int>(hist.at<float>(i));
            if (cumulativeCount >= saturatedPixels) {
                max = static_cast<double>(i);
                break;
            }
        }
    }

    if (min >= max) {
        min = 0.0;
        max = (image.type() == CV_16U) ? 65535.0 : ((image.type() == CV_32F) ? 1.0 : 255.0);
    }
}

/**
 * @brief (新) 计算并返回图像的直方图数据 (阶段 1 实现)
 */
QVector<double> ImageProcessor::calculateHistogram(const cv::Mat &image, int numBins, double minRange, double maxRange)
{
    if (image.empty() || numBins <= 0) {
        qWarning() << "calculateHistogram: Image is empty or numBins is invalid.";
        return QVector<double>();
    }

    // 确保范围有效
    if (minRange > maxRange) {
        qWarning() << "calculateHistogram: minRange > maxRange.";
        return QVector<double>();
    }

    // 处理 minRange == maxRange (例如纯色图像)
    if (qFuzzyCompare(minRange, maxRange)) {
        qWarning() << "calculateHistogram: minRange == maxRange (solid image?).";
        QVector<double> histData(numBins, 0.0);
        // 将所有像素计数放在第一个 bin
        histData[0] = static_cast<double>(image.total());
        return histData;
    }


    cv::Mat hist;
    int histSize = numBins;

    // cv::calcHist 要求 float 类型的范围 [min, max)
    float range[] = { static_cast<float>(minRange), static_cast<float>(maxRange) };

    int type = image.type();
    if (type == CV_8U || type == CV_16U || type == CV_16S || type == CV_32S) {
        // 对于整数类型, [minRange, maxRange] 对应的范围是 [minRange, maxRange + 1.0)
        range[1] = static_cast<float>(maxRange + 1.0);
    } else if (type == CV_32F || type == CV_64F) {
        // 对于浮点类型, [minRange, maxRange] 对应的范围是 [minRange, nextafter(maxRange))
        // 使用 nextafter 确保 maxRange 值能被包含在最后一个 bin 中
        range[1] = std::nextafter(static_cast<float>(maxRange), std::numeric_limits<float>::infinity());
    } else {
        // 未知类型，假设为整数
        qWarning() << "calculateHistogram: Unhandled cv::Mat type, assuming integer range.";
        range[1] = static_cast<float>(maxRange + 1.0);
    }

    const float* histRange = { range };
    int channels[] = { 0 };

    try {
        cv::calcHist(&image, 1, channels, cv::Mat(), // mask
                     hist, 1, &histSize, &histRange,
                     true, // uniform
                     false); // accumulate
    } catch (const cv::Exception& e) {
        qWarning() << "cv::calcHist failed:" << e.what();
        return QVector<double>();
    }

    // 将 cv::Mat (float 类型) 转换为 QVector<double>
    QVector<double> histData(numBins);
    for (int i = 0; i < numBins; ++i) {
        histData[i] = static_cast<double>(hist.at<float>(i));
    }

    return histData;
}

void ImageProcessor::calculateAutoWindowLevel(const QImage &image, int &min, int &max, double saturatedRatio)
{
    if (image.isNull() || !image.isGrayscale()) {
        min = 0;
        max = 255;
        return;
    }

    bool is16Bit = (image.format() == QImage::Format_Grayscale16);
    int maxVal = is16Bit ? 65535 : 255;
    QVector<int> histogram(maxVal + 1, 0);
    long long pixelCount = 0;

    for (int y = 0; y < image.height(); ++y) {
        if (is16Bit) {
            const quint16 *scanLine = reinterpret_cast<const quint16*>(image.constScanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                int val = scanLine[x];
                if (val >= 0 && val <= maxVal) {
                    histogram[val]++;
                }
                pixelCount++;
            }
        } else {
            const uchar *scanLine = image.constScanLine(y);
            for (int x = 0; x < image.width(); ++x) {
                int val = scanLine[x];
                histogram[val]++;
                pixelCount++;
            }
        }
    }

    int saturatedPixels = qRound(pixelCount * saturatedRatio);
    int cumulativeCount = 0;

    // 找到 min
    min = 0;
    for (int i = 0; i <= maxVal; ++i) {
        cumulativeCount += histogram[i];
        if (cumulativeCount >= saturatedPixels) {
            min = i;
            break;
        }
    }

    // 找到 max
    cumulativeCount = 0;
    max = maxVal;
    for (int i = maxVal; i >= 0; --i) {
        cumulativeCount += histogram[i];
        if (cumulativeCount >= saturatedPixels) {
            max = i;
            break;
        }
    }

    if (min >= max) {
        min = 0;
        max = maxVal;
    }
}

QImage ImageProcessor::applyWindowLevel(const QImage &originalImage, int min, int max)
{
    if (originalImage.isNull() || !originalImage.isGrayscale()) {
        return originalImage;
    }

    bool is16Bit = (originalImage.format() == QImage::Format_Grayscale16);
    int imageWidth = originalImage.width();
    int imageHeight = originalImage.height();
    QImage adjustedImage(imageWidth, imageHeight, QImage::Format_Grayscale8);

    double range = static_cast<double>(max - min);
    if (range <= 0) range = 1.0;

    for (int y = 0; y < imageHeight; ++y) {
        uchar *destLine = adjustedImage.scanLine(y);
        if (is16Bit) {
            const quint16 *srcLine = reinterpret_cast<const quint16*>(originalImage.constScanLine(y));
            for (int x = 0; x < imageWidth; ++x) {
                int val = srcLine[x];
                int newVal;
                if (val <= min) {
                    newVal = 0;
                } else if (val >= max) {
                    newVal = 255;
                } else {
                    newVal = qRound(((val - min) / range) * 255.0);
                }
                destLine[x] = static_cast<uchar>(newVal);
            }
        } else {
            const uchar *srcLine = originalImage.constScanLine(y);
            for (int x = 0; x < imageWidth; ++x) {
                int val = srcLine[x];
                int newVal;
                if (val <= min) {
                    newVal = 0;
                } else if (val >= max) {
                    newVal = 255;
                } else {
                    newVal = qRound(((val - min) / range) * 255.0);
                }
                destLine[x] = static_cast<uchar>(newVal);
            }
        }
    }

    return adjustedImage;
}

QImage ImageProcessor::applyWindowLevel(const cv::Mat &originalMat, double windowLevel, double windowWidth)
{
    if (originalMat.empty()) {
        return QImage();
    }

    double min = windowLevel - windowWidth / 2.0;
    double max = windowLevel + windowWidth / 2.0;

    if (min >= max) max = min + 0.001;

    double alpha = 255.0 / (max - min);
    double beta = -min * alpha;

    cv::Mat eightBitMat;
    originalMat.convertTo(eightBitMat, CV_8U, alpha, beta);

    return cvMat2QImage(eightBitMat);
}
