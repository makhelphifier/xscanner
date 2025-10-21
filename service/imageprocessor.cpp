// service/imageprocessor.cpp

#include "imageprocessor.h"
#include <QVector>
#include <QtMath>
#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <QDataStream>
#include <string>
#include <windows.h> // 确保包含Windows头文件
#include <cstdio>  // 包含 fopen, fclose 等函数
#include <cwchar>  // 包含 _wfopen_s
#include <fstream>
#include <iostream>

ImageProcessor::ImageProcessor() {}




QImage ImageProcessor::readRawImg_qImage(const QString imgPath, const int width, const int height)
{
    const std::string filename = imgPath.toStdString();
    cv::Mat image(height, width, CV_16UC1);// 创建一个空的Mat对象，用于存储16位无符号整数数据

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return QImage();
    }

    // 读取数据到Mat对象
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue;
            file.read(reinterpret_cast<char*>(&pixelValue), sizeof(pixelValue));
            image.at<uint16_t>(y, x) = pixelValue;
        }
    }
    file.close();


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


QImage ImageProcessor::loadRaw16bitImage(const QString &filePath, int width, int height)
{
    qDebug() << "--- [loadRaw16bitImage] Function started ---";
    qDebug() << "File path:" << filePath;
    qDebug() << "Expected dimensions:" << width << "x" << height;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "!!!!!! CRITICAL: Failed to open raw file:" << filePath << "!!!!!!";
        return QImage();
    }
    qDebug() << "File opened successfully.";

    // 一次性读取所有原始二进制数据
    QByteArray buffer = file.readAll();
    file.close();
    qDebug() << "Read" << buffer.size() << "bytes from file. File closed.";

    // 校验文件大小是否符合预期
    const qint64 expectedSize = static_cast<qint64>(width) * height * sizeof(quint16);
    if (buffer.size() != expectedSize) {
        qDebug() << "!!!!!! CRITICAL: File size mismatch! Expected:" << expectedSize << "Got:" << buffer.size() << "!!!!!!";
        return QImage();
    }

    // 将字节数组的指针重新解释为 16 位无符号整数指针
    // 这是一种高效的方式，避免了逐个像素的读取和数据拷贝
    const quint16* pixelData = reinterpret_cast<const quint16*>(buffer.constData());

    const int numPixels = width * height;
    qDebug() << "Total number of pixels to process:" << numPixels;

    quint16 minVal = 65535;
    quint16 maxVal = 0;

    qDebug() << "Starting to find min/max values...";
    // 遍历数据以找到最大值和最小值
    for (int i = 0; i < numPixels; ++i) {
        if (pixelData[i] < minVal) minVal = pixelData[i];
        if (pixelData[i] > maxVal) maxVal = pixelData[i];
    }
    qDebug() << "Min value found:" << minVal << ", Max value found:" << maxVal;

    // 创建8位灰度图用于显示
    QImage image(width, height, QImage::Format_Grayscale8);
    if (image.isNull()) {
        qDebug() << "!!!!!! CRITICAL: Failed to create QImage object! !!!!!!";
        return QImage();
    }
    qDebug() << "QImage object created successfully.";
    uchar *bits = image.bits();

    float range = maxVal - minVal;
    if (range == 0) range = 1.0f;
    qDebug() << "Normalization range calculated:" << range;

    qDebug() << "Starting 16-bit to 8-bit normalization...";
    // 将16位数据归一化到8位
    for (int i = 0; i < numPixels; ++i) {
        bits[i] = static_cast<uchar>(((pixelData[i] - minVal) / range) * 255.0f);
    }
    qDebug() << "Normalization finished.";

    qDebug() << "--- [loadRaw16bitImage] Function finished, returning image. ---";
    // QImage 是隐式共享的，直接返回即可，无需 copy()
    return image;
}

void ImageProcessor::calculateAutoWindowLevel(const QImage &image, int &min, int &max, double saturatedRatio)
{
    if (image.isNull() || !image.isGrayscale()) {
        min = 0;
        max = 255;
        return;
    }

    QVector<int> histogram(256, 0);
    long pixelCount = 0;

    for (int y = 0; y < image.height(); ++y) {
        const uchar *scanLine = image.constScanLine(y);
        for (int x = 0; x < image.width(); ++x) {
            histogram[scanLine[x]]++;
            pixelCount++;
        }
    }

    int saturatedPixels = qRound(pixelCount * saturatedRatio);
    int cumulativeCount = 0;

    // 找到 min
    min = 0;
    for (int i = 0; i < 256; ++i) {
        cumulativeCount += histogram[i];
        if (cumulativeCount >= saturatedPixels) {
            min = i;
            break;
        }
    }

    // 找到 max
    cumulativeCount = 0;
    max = 255;
    for (int i = 255; i >= 0; --i) {
        cumulativeCount += histogram[i];
        if (cumulativeCount >= saturatedPixels) {
            max = i;
            break;
        }
    }

    if (min >= max) {
        min = 0;
        max = 255;
    }
}

QImage ImageProcessor::applyWindowLevel(const QImage &originalImage, int min, int max)
{
    if (originalImage.isNull() || !originalImage.isGrayscale()) {
        return originalImage;
    }

    QImage newImage = originalImage.copy();
    QVector<QRgb> lut(256);
    double range = max - min;
    if (range <= 0) range = 1.0;

    for (int i = 0; i < 256; ++i) {
        int value;
        if (i <= min) {
            value = 0;
        } else if (i >= max) {
            value = 255;
        } else {
            value = qRound(((i - min) / range) * 255.0);
        }
        lut[i] = qRgb(value, value, value);
    }

    newImage.setColorTable(lut);
    return newImage;
}


QImage ImageProcessor::loadRawImage(const QString &filePath, int width, int height)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open raw file:" << filePath;
        return QImage();
    }

    QByteArray rawData = file.readAll();
    file.close();

    // 假设是 8 位灰度图
    if (rawData.size() != width * height) {
        qDebug() << "Raw file size" << rawData.size() << "does not match expected dimensions" << width * height;
        return QImage();
    }

    // 创建一个 QImage 并将数据复制进去
    // QImage 的这个构造函数会复制数据，所以 rawData 可以在栈上安全地销毁
    QImage image(reinterpret_cast<const uchar*>(rawData.constData()), width, height, QImage::Format_Grayscale8);

    // 由于 QImage 的构造函数可能不会复制数据，而是引用它，
    // 为了安全起见，我们显式地创建一个深拷贝。
    return image.copy();
}


