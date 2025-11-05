// gui/viewmodels/imageviewmodel.cpp

#include "imageviewmodel.h"
#include "service/imageprocessor.h"
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <QtGlobal> // for qBound

ImageViewModel::ImageViewModel(QObject *parent)
    : QObject(parent)
{
}

// --- 命令实现 ---

void ImageViewModel::loadImage(const QString &filePath)
{
    // 重写加载逻辑
    m_originalImageMat = cv::Mat();
    m_bitDepth = 8; // 默认 8 位

    if (filePath.endsWith(".fraw", Qt::CaseInsensitive)) { // 假设 .fraw 是 32 位浮点
        m_originalImageMat = ImageProcessor::readRawImg_cvMat(filePath, 2882, 2340, CV_32F);
        m_bitDepth = 32;
    } else if (filePath.endsWith(".raw", Qt::CaseInsensitive) || filePath.endsWith(".bin", Qt::CaseInsensitive)) {
        m_originalImageMat = ImageProcessor::readRawImg_cvMat(filePath, 2882, 2340, CV_16U);
        m_bitDepth = 16;
    } else {
        // 加载标准图像 (JPG, PNG 等)
        QImage loadedImage(filePath);
        if (!loadedImage.isNull()) {
            // 调用 QImage2cvMat
            m_originalImageMat = ImageProcessor::QImage2cvMat(loadedImage);

            // 转换为单通道灰度
            if (m_originalImageMat.channels() > 1) {
                cv::cvtColor(m_originalImageMat, m_originalImageMat, cv::COLOR_BGR2GRAY);
            }

            // 确保 8 位无符号
            m_originalImageMat.convertTo(m_originalImageMat, CV_8UC1);
            m_bitDepth = 8;
        }
    }

    m_originalDataType = m_originalImageMat.type();

    if (m_originalImageMat.empty()) {
        m_imageBounds = QRectF();
        emit imageLoaded(0.0, 0.0, 0, QRectF()); // 发出空信号
        return;
    }

    m_imageBounds = QRectF(0, 0, m_originalImageMat.cols, m_originalImageMat.rows);

    // 计算初始范围
    ImageProcessor::calculateAutoWindowLevel(m_originalImageMat, m_dataMin, m_dataMax, 0.0035); // 使用 0.35% 饱和度

    // 初始化窗宽窗位
    m_windowWidth = m_dataMax - m_dataMin;
    m_windowLevel = m_dataMin + m_windowWidth / 2.0;

    // 发出 imageLoaded 信号，通知 UI 更新范围
    emit imageLoaded(m_dataMin, m_dataMax, m_bitDepth, m_imageBounds);

    // 默认启用自动窗宽窗位
    setAutoWindowing(true);
}

void ImageViewModel::setWindowWidth(double width)
{
    // 使用 m_dataMax
    double newWidth = qBound(0.001, width, (m_dataMax - m_dataMin) + 1.0);

    if (m_windowWidth == newWidth) return;

    m_windowWidth = newWidth;
    m_autoWindowing = false; // 手动调整

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
    emit autoWindowingChanged(false);
}

void ImageViewModel::setLevel(double level)
{
    // 使用 m_dataMin/Max
    double newLevel = qBound(m_dataMin, level, m_dataMax);

    if (m_windowLevel == newLevel) return;

    m_windowLevel = newLevel;
    m_autoWindowing = false; // 手动调整

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
    emit autoWindowingChanged(false);
}

void ImageViewModel::setAutoWindowing(bool enabled)
{
    if (m_autoWindowing == enabled && enabled) return; // 避免重复计算

    m_autoWindowing = enabled;
    emit autoWindowingChanged(enabled);

    if (m_originalImageMat.empty()) return;

    if (enabled) {
        double min, max;
        calculateAutoWindowLevel(min, max);
        m_windowWidth = max - min;
        m_windowLevel = min + m_windowWidth / 2.0;
    } else {
        // 恢复全范围
        m_windowWidth = m_dataMax - m_dataMin;
        m_windowLevel = m_dataMin + m_windowWidth / 2.0;
    }

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
}

// --- 查询实现 ---

void ImageViewModel::requestPixelInfo(const QPointF &scenePos)
{
    int x = qRound(scenePos.x());
    int y = qRound(scenePos.y());
    double value = -1.0; // N/A 默认

    if (m_imageBounds.contains(scenePos) && x >= 0 && x < m_originalImageMat.cols && y >= 0 && y < m_originalImageMat.rows) {
        value = getPixelValue(x, y);
    }

    emit pixelInfoReady(x, y, value);
}

double ImageViewModel::getPixelValue(int x, int y) const
{
    if (m_originalImageMat.empty()) {
        return -1.0;
    }

    // 根据类型返回
    switch(m_originalDataType) {
    case CV_8UC1:
        return static_cast<double>(m_originalImageMat.at<uchar>(y, x));
    case CV_16UC1:
        return static_cast<double>(m_originalImageMat.at<quint16>(y, x));
    case CV_16SC1:
        return static_cast<double>(m_originalImageMat.at<qint16>(y, x));
    case CV_32SC1:
        return static_cast<double>(m_originalImageMat.at<qint32>(y, x));
    case CV_32FC1:
        return static_cast<double>(m_originalImageMat.at<float>(y, x));
    case CV_64FC1:
        return m_originalImageMat.at<double>(y, x);
    default:
        return -1.0;
    }
}

// --- 属性访问器 ---

double ImageViewModel::currentWindowWidth() const { return m_windowWidth; }
double ImageViewModel::currentWindowLevel() const { return m_windowLevel; }
int ImageViewModel::bitDepth() const { return m_bitDepth; }
QRectF ImageViewModel::imageBounds() const { return m_imageBounds; }
bool ImageViewModel::isAutoWindowing() const { return m_autoWindowing; }

// --- 内部逻辑 ---

void ImageViewModel::applyWindowLevel()
{
    if (m_originalImageMat.empty()) {
        emit pixmapChanged(QPixmap());
        return;
    }

    // 调用新版 ImageProcessor
    QImage adjustedImage = ImageProcessor::applyWindowLevel(m_originalImageMat, m_windowLevel, m_windowWidth);
    emit pixmapChanged(QPixmap::fromImage(adjustedImage));
}

void ImageViewModel::calculateAutoWindowLevel(double &min, double &max)
{
    ImageProcessor::calculateAutoWindowLevel(m_originalImageMat, min, max);
    // 将计算出的饱和范围也存起来
    m_dataMin = min;
    m_dataMax = max;
}
