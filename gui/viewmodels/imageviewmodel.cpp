#include "imageviewmodel.h"
#include "service/imageprocessor.h"
#include <QDebug>

ImageViewModel::ImageViewModel(QObject *parent)
    : QObject(parent)
{
}

// --- 命令实现 ---

void ImageViewModel::loadImage(const QString &filePath)
{
    QImage loadedImage;
    if (filePath.endsWith(".raw", Qt::CaseInsensitive) || filePath.endsWith(".bin", Qt::CaseInsensitive)) {
        loadedImage = ImageProcessor::readRawImg_qImage(filePath, 2882, 2340); //
    } else {
        loadedImage.load(filePath);
    }

    if (loadedImage.isNull()) {
        m_originalImage = QImage();
        m_imageBounds = QRectF();
        emit imageLoaded(0, 0, 0, QRectF()); // 发出空信号
        return;
    }

    // 检测位深
    m_bitDepth = (loadedImage.format() == QImage::Format_Grayscale16) ? 16 : 8;
    if (!loadedImage.isGrayscale()) {
        loadedImage = loadedImage.convertToFormat(QImage::Format_Grayscale8);
        m_bitDepth = 8;
    }

    m_originalImage = loadedImage;
    m_imageBounds = m_originalImage.rect();

    // 初始化窗宽窗位
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    m_windowWidth = maxVal + 1;
    m_windowLevel = maxVal / 2;

    // 发出 imageLoaded 信号，通知 UI 更新范围
    emit imageLoaded(0, maxVal, m_bitDepth, m_imageBounds);

    // 默认启用自动窗宽窗位
    setAutoWindowing(true);
}

void ImageViewModel::setWindowWidth(int width)
{
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    int newWidth = qBound(1, width, maxVal + 1);

    if (m_windowWidth == newWidth) return;

    m_windowWidth = newWidth;
    m_autoWindowing = false; // 手动调整

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
    emit autoWindowingChanged(false);
}

void ImageViewModel::setLevel(int level)
{
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    int newLevel = qBound(0, level, maxVal);

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

    if (m_originalImage.isNull()) return;

    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    if (enabled) {
        int min, max;
        calculateAutoWindowLevel(min, max);
        m_windowWidth = max - min;
        m_windowLevel = min + m_windowWidth / 2;
    } else {
        // 恢复全范围
        m_windowWidth = maxVal + 1;
        m_windowLevel = maxVal / 2;
    }

    applyWindowLevel();
    emit windowLevelChanged(m_windowWidth, m_windowLevel);
}

// --- 查询实现 ---

void ImageViewModel::requestPixelInfo(const QPointF &scenePos)
{
    int x = qRound(scenePos.x());
    int y = qRound(scenePos.y());
    int value = -1; // N/A 默认

    if (m_imageBounds.contains(scenePos) && m_originalImage.valid(x, y)) {
        value = getPixelValue(x, y);
    }

    emit pixelInfoReady(x, y, value);
}

int ImageViewModel::getPixelValue(int x, int y) const
{
    if (m_originalImage.isNull() || !m_originalImage.valid(x, y)) {
        return -1;
    }

    if (m_bitDepth == 16) {
        const quint16* scanLine = reinterpret_cast<const quint16*>(m_originalImage.constScanLine(y));
        return scanLine[x];
    } else {
        return qGray(m_originalImage.pixel(x, y));
    }
}

// --- 属性访问器 ---

int ImageViewModel::currentWindowWidth() const { return m_windowWidth; }
int ImageViewModel::currentWindowLevel() const { return m_windowLevel; }
int ImageViewModel::bitDepth() const { return m_bitDepth; }
QRectF ImageViewModel::imageBounds() const { return m_imageBounds; }
bool ImageViewModel::isAutoWindowing() const { return m_autoWindowing; }

// --- 内部逻辑 ---

void ImageViewModel::applyWindowLevel()
{
    if (m_originalImage.isNull()) {
        emit pixmapChanged(QPixmap());
        return;
    }

    int min = m_windowLevel - m_windowWidth / 2;
    int max = m_windowLevel + m_windowWidth / 2;
    int maxVal = (m_bitDepth == 16) ? 65535 : 255;
    min = qBound(0, min, maxVal);
    max = qBound(0, max, maxVal);
    if (min > max) std::swap(min, max);

    QImage adjustedImage = ImageProcessor::applyWindowLevel(m_originalImage, min, max);
    emit pixmapChanged(QPixmap::fromImage(adjustedImage));
}

void ImageViewModel::calculateAutoWindowLevel(int &min, int &max)
{
    ImageProcessor::calculateAutoWindowLevel(m_originalImage, min, max);
}
