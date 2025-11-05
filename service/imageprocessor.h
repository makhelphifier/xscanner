// gui/service/imageprocessor.h

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

    // --- cv::Mat (浮点) 版本 ---
    static void calculateAutoWindowLevel(const cv::Mat &image, double &min, double &max, double saturatedRatio = 0.0035);
    static QImage applyWindowLevel(const cv::Mat &originalMat, double windowLevel, double windowWidth);
    static cv::Mat readRawImg_cvMat(const QString imgPath, const int width, const int height, int cvType);

    static cv::Mat QImage2cvMat(const QImage &image);
    static QImage cvMat2QImage(const cv::Mat &mat);


    // --- QImage (整型) 版本 ---
    static void calculateAutoWindowLevel(const QImage &image, int &min, int &max, double saturatedRatio = 0.0035);
    static QImage applyWindowLevel(const QImage &originalImage, int min, int max);
    static QImage readRawImg_qImage(const QString imgPath, const int width, const int height);

};
#endif // IMAGEPROCESSOR_H
