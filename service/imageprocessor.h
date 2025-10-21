// service/imageprocessor.h

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <QVector>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <QImage>

class ImageProcessor
{
public:
    ImageProcessor();

    // 计算自动窗宽窗位
    static void calculateAutoWindowLevel(const QImage &image, int &min, int &max, double saturatedRatio = 0.0035);

    // 应用窗宽窗位并返回新图像
    static QImage applyWindowLevel(const QImage &originalImage, int min, int max);

    static QImage loadRawImage(const QString &filePath, int width, int height);
    static QImage loadRaw16bitImage(const QString &filePath, int width, int height); // 添加此行


    static QImage readRawImg_qImage(const QString imgPath, const int width, const int height);
    static QImage cvMat2QImage(const cv::Mat &mat);
};

#endif // IMAGEPROCESSOR_H
