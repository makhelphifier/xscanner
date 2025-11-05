#ifndef IMAGEVIEWMODEL_H
#define IMAGEVIEWMODEL_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QRectF>

class ImageViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ImageViewModel(QObject *parent = nullptr);
    virtual ~ImageViewModel() = default;

    // --- 属性访问器 ---
    int currentWindowWidth() const;
    int currentWindowLevel() const;
    int bitDepth() const;
    QRectF imageBounds() const;
    bool isAutoWindowing() const;

public slots:
    // --- 命令 (从 View 调用) ---
    void loadImage(const QString &filePath);
    void setWindowWidth(int width);
    void setLevel(int level);
    void setAutoWindowing(bool enabled);

    // --- 查询 (从 View 调用) ---
    void requestPixelInfo(const QPointF &scenePos);
    int getPixelValue(int x, int y) const; // 公开，供 PointMeasureItem 等使用

signals:
    // --- 通知 (View 监听这些信号) ---

    /**
     * @brief 当显示的 QPixmap 更新时发出 (例如加载或窗宽窗位更改后)
     */
    void pixmapChanged(const QPixmap &pixmap);

    /**
     * @brief 当图像加载成功或失败时发出，用于更新 UI 范围
     * @param min 灰度最小值 (通常为 0)
     * @param max 灰度最大值 (255 或 65535)
     * @param bits 位深 (8 或 16)
     * @param imageRect 图像尺寸
     */
    void imageLoaded(int min, int max, int bits, QRectF imageRect);

    /**
     * @brief 当窗宽窗位值改变时发出 (无论是自动还是手动)
     */
    void windowLevelChanged(int width, int level);

    /**
     * @brief 当自动窗宽窗位状态切换时发出
     */
    void autoWindowingChanged(bool enabled);

    /**
     * @brief 响应 requestPixelInfo，返回计算后的像素信息
     */
    void pixelInfoReady(int x, int y, int value);

private:
    // --- 内部逻辑 ---
    void applyWindowLevel();
    void calculateAutoWindowLevel(int &min, int &max);

    // --- 数据模型和状态 ---
    QImage m_originalImage; // Model
    int m_bitDepth = 8;
    int m_windowWidth = 256;
    int m_windowLevel = 128;
    bool m_autoWindowing = false;
    QRectF m_imageBounds;
};

#endif // IMAGEVIEWMODEL_H
