#include "imageprocessor.h"
#include <QVector>
#include <QtMath>
#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <QDataStream>
#include <windows.h>
#include <iostream>

ImageProcessor::ImageProcessor() {}


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
    break;
    // 8位无符号，3通道
    case CV_8UC3:
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    break;
    // 8位无符号，4通道
    case CV_8UC4:
    {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    break;
    case CV_16UC1:
    {
        QImage image(reinterpret_cast<const uchar*>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_Grayscale16);
        return image.copy();
    }
    break;
    default:
        break;
    }
    return QImage();
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
