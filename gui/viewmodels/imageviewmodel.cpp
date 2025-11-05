// gui/viewmodels/imageviewmodel.cpp

#include "imageviewmodel.h"
#include "service/imageprocessor.h"
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <QtGlobal> // for qBound
#include <QtMath> // for qFuzzyCompare
#include <QMap>

ImageViewModel::ImageViewModel(QObject *parent)
    : QObject(parent),
    m_histogramKeyRange(0, 0)
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

        // 发出空直方图
        m_histogramData.clear();
        m_histogramKeyRange = QCPRange(0, 0);
        emit histogramDataReady(m_histogramData, m_histogramKeyRange);
        return;
    }

    m_imageBounds = QRectF(0, 0, m_originalImageMat.cols, m_originalImageMat.rows);

    // 获取并存储 *真实* 的完整数据范围
    cv::minMaxLoc(m_originalImageMat, &m_trueDataMin, &m_trueDataMax);

    // 初始化默认曲线
    m_curvePoints.clear();
    m_curvePoints.insert(m_trueDataMin, 0.0);
    m_curvePoints.insert(m_trueDataMax, 255.0);

    // 计算直方图
    m_histogramData = ImageProcessor::calculateHistogram(m_originalImageMat, HISTOGRAM_BINS, m_trueDataMin, m_trueDataMax);
    m_histogramKeyRange = QCPRange(m_trueDataMin, m_trueDataMax);
    emit histogramDataReady(m_histogramData, m_histogramKeyRange);

    // 计算并存储 *饱和* 范围 (用于自动窗宽窗位)
    // m_dataMin/Max 现在存储的是自动窗宽的边界
    ImageProcessor::calculateAutoWindowLevel(m_originalImageMat, m_dataMin, m_dataMax, 0.0035);

    // 初始化窗宽窗位 (使用自动值)
    m_windowWidth = m_dataMax - m_dataMin;
    m_windowLevel = m_dataMin + m_windowWidth / 2.0;

    // 发出 imageLoaded 信号，UI 滑块应使用 *真实* 范围
    emit imageLoaded(m_trueDataMin, m_trueDataMax, m_bitDepth, m_imageBounds);

    // 默认启用自动窗宽窗位
    setAutoWindowing(true);
}

void ImageViewModel::setWindowWidth(double width)
{
    setAdjustmentMode(ModeWindowLevel);

    // 使用 *真实* 范围来约束手动输入
    double newWidth = qBound(0.001, width, (m_trueDataMax - m_trueDataMin) + 1.0);
    // 使用 qFuzzyCompare 比较浮点数
    // if (qFuzzyCompare(m_windowWidth, newWidth)) return;

    m_windowWidth = newWidth;
    m_autoWindowing = false; // 手动调整

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
    emit autoWindowingChanged(false);
}

void ImageViewModel::setLevel(double level)
{
    setAdjustmentMode(ModeWindowLevel);

    // 使用 *真实* 范围来约束手动输入
    double newLevel = qBound(m_trueDataMin, level, m_trueDataMax);

    // 使用 qFuzzyCompare 比较浮点数
    // if (qFuzzyCompare(m_windowLevel, newLevel)) return;

    m_windowLevel = newLevel;
    m_autoWindowing = false; // 手动调整

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
    emit autoWindowingChanged(false);
}

void ImageViewModel::setAutoWindowing(bool enabled)
{
    // 如果启用，确保模式为 WindowLevel
    if (enabled) {
        setAdjustmentMode(ModeWindowLevel);
    }

    // 状态未改变，直接返回
    if (m_autoWindowing == enabled) return;

    m_autoWindowing = enabled;
    emit autoWindowingChanged(enabled);

    if (m_originalImageMat.empty()) return;

    // --- 修改：重构逻辑 ---
    if (enabled) {
        // 如果 *开启* 自动窗宽

        // 1. 重新计算自动(饱和)范围
        calculateAutoWindowLevel(m_dataMin, m_dataMax);
        m_windowWidth = m_dataMax - m_dataMin;
        m_windowLevel = m_dataMin + m_windowWidth / 2.0;

        // 2. 应用并发出新值
        applyWindowLevel();
        emit windowLevelChanged(m_windowWidth, m_windowLevel);

    } else {
        // 如果 *关闭* 自动窗宽
        // (例如：用户取消了复选框)

        // 3. [关键]：我们什么也不做。
        // 当前的 m_windowWidth 和 m_windowLevel 被保留，
        // 它们现在被视为“手动”值。
    }
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
    ImageProcessor::calculateAutoWindowLevel(m_originalImageMat, min, max, 0.0035);
    // 将计算出的饱和范围也存起来
    m_dataMin = min;
    m_dataMax = max;
}

// --- 模式和曲线控制槽的实现 ---

/**
 * @brief 设置当前的图像调整模式 (窗宽窗位 或 曲线)
 */
void ImageViewModel::setAdjustmentMode(AdjustmentMode mode)
{
    if (m_adjustmentMode == mode)
        return;

    m_adjustmentMode = mode;
    emit adjustmentModeChanged(m_adjustmentMode);

    // 如果切换到曲线模式，则“自动窗宽”必须关闭
    if (m_adjustmentMode == ModeCurves) {
        if (m_autoWindowing) { // 仅在状态改变时才设置和发出信号
            m_autoWindowing = false;
            emit autoWindowingChanged(false);
        }
        // 切换到曲线模式时，立即发送当前曲线点
        emit curvePointsChanged(m_curvePoints);
    }

    // (TODO: 在未来，这里应该调用 applyAdjustment() 来更新图像)
    // applyWindowLevel(); // 暂时保留，直到曲线应用逻辑被添加
}

/**
 * @brief 添加一个曲线锚点
 */
void ImageViewModel::addCurvePoint(double key, double value)
{
    // 确保值在有效输出范围内
    value = qBound(0.0, value, 255.0);
    key = qBound(m_trueDataMin, key, m_trueDataMax);

    m_curvePoints.insert(key, value);
    emit curvePointsChanged(m_curvePoints);

    // (TODO: 在未来，这里应该调用 applyAdjustment())
}

/**
 * @brief 移动一个曲线锚点
 * @param oldKey 锚点原来的 Key (用于查找)
 * @param newKey 锚点的新 Key
 * @param newValue 锚点的新 Value
 */
void ImageViewModel::moveCurvePoint(double oldKey, double newKey, double newValue)
{
    if (!m_curvePoints.contains(oldKey))
        return;

    // 确保值在有效输出范围内
    newValue = qBound(0.0, newValue, 255.0);
    newKey = qBound(m_trueDataMin, newKey, m_trueDataMax);

    // 不允许移动端点
    if (qFuzzyCompare(oldKey, m_curvePoints.firstKey()) || qFuzzyCompare(oldKey, m_curvePoints.lastKey())) {
        newKey = oldKey; // 保持 Key 不变，只允许修改 Value
    }

    m_curvePoints.remove(oldKey);
    m_curvePoints.insert(newKey, newValue);

    emit curvePointsChanged(m_curvePoints);

    // (TODO: 在未来，这里应该调用 applyAdjustment())
}

/**
 * @brief 移除一个曲线锚点
 * @param key 要移除的锚点的 Key
 */
void ImageViewModel::removeCurvePoint(double key)
{
    if (!m_curvePoints.contains(key))
        return;

    // 不允许移除第一个或最后一个锚点
    if (qFuzzyCompare(key, m_curvePoints.firstKey()) || qFuzzyCompare(key, m_curvePoints.lastKey())) {
        return;
    }

    m_curvePoints.remove(key);
    emit curvePointsChanged(m_curvePoints);

    // (TODO: 在未来，这里应该调用 applyAdjustment())
}

/**
 * @brief 将曲线重置为默认的直线状态
 */
void ImageViewModel::resetCurve()
{
    m_curvePoints.clear();
    m_curvePoints.insert(m_trueDataMin, 0.0);
    m_curvePoints.insert(m_trueDataMax, 255.0);
    emit curvePointsChanged(m_curvePoints);

    // (TODO: 在未来，这里应该调用 applyAdjustment())
}
