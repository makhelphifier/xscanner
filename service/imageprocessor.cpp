// service/imageprocessor.cpp

#include "imageprocessor.h"
#include <QVector>
#include <QtMath>
#include <QFile>     // <-- 添加此行
#include <QDebug>    // <-- 添加此行

ImageProcessor::ImageProcessor() {}

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


