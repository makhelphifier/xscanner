#ifndef IMAGEVIEWMODEL_H
#define IMAGEVIEWMODEL_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QRectF>
#include <QVector>
#include <QMap>
#include <opencv2/core.hpp>
#include "qcustomplot.h"

    class QCPRange;

class ImageViewModel : public QObject
{
    Q_OBJECT

public:
    enum AdjustmentMode {
        ModeWindowLevel,
        ModeCurves
    };
    Q_ENUM(AdjustmentMode)

    explicit ImageViewModel(QObject *parent = nullptr);
    virtual ~ImageViewModel() = default;

    // --- 属性访问器 ---
    double currentWindowWidth() const;
    double currentWindowLevel() const;
    int bitDepth() const;
    QRectF imageBounds() const;
    bool isAutoWindowing() const;

public slots:
    // --- 命令 (从 View 调用) ---
    void loadImage(const QString &filePath);
    void setWindowWidth(double width);
    void setLevel(double level);
    void setAutoWindowing(bool enabled);

    // --- 查询 (从 View 调用) ---
    void requestPixelInfo(const QPointF &scenePos);
    double getPixelValue(int x, int y) const;

    // --- 模式和曲线控制槽 ---
    void setAdjustmentMode(AdjustmentMode mode);
    void addCurvePoint(double key, double value);
    void moveCurvePoint(double oldKey, double newKey, double newValue);
    void removeCurvePoint(double key);
    void resetCurve();

signals:
    // --- 通知 (View 监听这些信号) ---

    /**
     * @brief 当显示的 QPixmap 更新时发出 (例如加载或窗宽窗位更改后)
     */
    void pixmapChanged(const QPixmap &pixmap);

    /**
     * @brief 当图像加载成功或失败时发出，用于更新 UI 范围
     * min/max 现在是 double 类型，用于设置 SpinBox
     */
    void imageLoaded(double min, double max, int bits, QRectF imageRect);

    /**
     * @brief 当窗宽窗位值改变时发出 (无论是自动还是手动)
     */
    void windowLevelChanged(double width, double level);

    /**
     * @brief 当自动窗宽窗位状态切换时发出
     */
    void autoWindowingChanged(bool enabled);

    /**
     * @brief 响应 requestPixelInfo，返回计算后的像素信息
     */
    void pixelInfoReady(int x, int y, double value);

    /**
     * @brief 当直方图数据计算完成时发出
     * @param data 包含 HISTOGRAM_BINS 个计数值的 QVector
     * @param keyRange 数据的 X 轴范围 (m_trueDataMin 到 m_trueDataMax)
     */
    void histogramDataReady(const QVector<double> &data, const QCPRange &keyRange);

    /**
     * @brief 当模式从 WW/L 切换到 Curves 时发出
     */
    void adjustmentModeChanged(AdjustmentMode newMode);
    /**
     * @brief 当锚点被添加、移动或删除时发出
     */
    void curvePointsChanged(const QMap<double, double> &points);

private:
    // --- 内部逻辑 ---
    void applyWindowLevel();
    void calculateAutoWindowLevel(double &min, double &max);

    // --- 数据模型和状态 ---
    static const int HISTOGRAM_BINS = 256;

    cv::Mat m_originalImageMat;
    int m_originalDataType;     // CV_16U, CV_32F etc.
    int m_bitDepth = 8;         // 仅用于显示 (8, 16, 32)
    double m_windowWidth = 256.0;
    double m_windowLevel = 128.0;
    double m_dataMin = 0.0;       // 数据的饱和最小值 (自动窗宽窗位范围)
    double m_dataMax = 255.0;     // 数据的饱和最大值 (自动窗宽窗位范围)
    double m_trueDataMin = 0.0;   // 数据的真实最小值 (完整范围)
    double m_trueDataMax = 255.0; // 数据的真实最大值 (完整范围)
    bool m_autoWindowing = false;
    QRectF m_imageBounds;

    // --- 调整模式和曲线数据 ---
    AdjustmentMode m_adjustmentMode = ModeWindowLevel;
    QMap<double, double> m_curvePoints;

    // --- 直方图数据 ---
    QVector<double> m_histogramData;
    QCPRange m_histogramKeyRange;
};

#endif // IMAGEVIEWMODEL_H
