// service/imageprocessor.cpp

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
