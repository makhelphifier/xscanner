#include "imageprocessor.h"
#include <QVector>
#include <QtMath>
#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <QDataStream>
#include <string>
#include <windows.h>
#include <fstream>
#include <iostream>

ImageProcessor::ImageProcessor() {}

QImage ImageProcessor::readRawImg_qImage(const QString imgPath, const int width, const int height)
{
    const std::string filename = imgPath.toStdString();
    // 创建一个Mat对象，用于存储16位无符号整数数据
    cv::Mat image(height, width, CV_16UC1);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return QImage();
    }

    // 为提高效率，一次性将数据块读入Mat对象
    file.read(reinterpret_cast<char*>(image.data), static_cast<std::streamsize>(width) * height * sizeof(uint16_t));
    file.close();

    // --- 将图像归一化以用于显示 ---
    // 创建一个新的Mat对象用于存放8位图像
    cv::Mat normalizedImage;
    // 将16位图像的灰度范围（从实际最小值到最大值）拉伸到0-255，并转换为8位图像
    // 这会让图像使用完整的灰度范围进行显示
    cv::normalize(image, normalizedImage, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    // 将归一化后的8位Mat转换为QImage并返回
    return cvMat2QImage(normalizedImage);
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
