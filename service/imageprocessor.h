#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <QVector>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

class ImageProcessor
{
public:
    ImageProcessor();

    // --- cv::Mat (浮点) 版本 ---
    static void calculateAutoWindowLevel(const cv::Mat &image, double &min, double &max, double saturatedRatio = 0.0035);
    static QImage applyWindowLevel(const cv::Mat &originalMat, double windowLevel, double windowWidth);
    static cv::Mat readRawImg_cvMat(const QString imgPath, const int width, const int height, int cvType);

    /**
     * @brief (新) 计算并返回图像的直方图数据
     * @param image 原始 cv::Mat (CV_16U, CV_32F 等)
     * @param numBins 要分成的箱数 (例如 256)
     * @param minRange 数据的真实最小值 (m_trueDataMin)
     * @param maxRange 数据的真实最大值 (m_trueDataMax)
     * @return 一个 QVector<double>，包含 numBins 个元素，代表每个箱的像素计数
     */
    static QVector<double> calculateHistogram(const cv::Mat &image, int numBins, double minRange, double maxRange);

    static cv::Mat QImage2cvMat(const QImage &image);
    static QImage cvMat2QImage(const cv::Mat &mat);


    // --- QImage (整型) 版本 ---
    static void calculateAutoWindowLevel(const QImage &image, int &min, int &max, double saturatedRatio = 0.0035);
    static QImage applyWindowLevel(const QImage &originalImage, int min, int max);
    static QImage readRawImg_qImage(const QString imgPath, const int width, const int height);

};

#endif // IMAGEPROCESSOR_H
