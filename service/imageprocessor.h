// service/imageprocessor.h

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <QVector>

class ImageProcessor
{
public:
    ImageProcessor();

    // 计算自动窗宽窗位
    static void calculateAutoWindowLevel(const QImage &image, int &min, int &max, double saturatedRatio = 0.0035);

    // 应用窗宽窗位并返回新图像
    static QImage applyWindowLevel(const QImage &originalImage, int min, int max);

    static QImage loadRawImage(const QString &filePath, int width, int height);
};

#endif // IMAGEPROCESSOR_H
