#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QMouseEvent>
#include <QImage>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
// #include "gui/states/drawingstatemachine.h"
#include <QRectF>

class QGraphicsRectItem;
class QGraphicsLineItem;
class AnnotationPointItem;
class DrawingStateMachine;
class ROI;
class ExtractedImageViewer;


class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void loadImage(const QString &filePath);
    QGraphicsPixmapItem *pixmapItem() const { return m_pixmapItem; }
    enum ToolMode {
        ModeSelect,       // 用于平移、拖动 Handle
        ModeDrawRect,     // 绘制矩形 (使用 RectROI)
        ModeDrawEllipse,   // 绘制椭圆 (使用 AnnotationEllipseItem)
        ModeDrawLine,
        ModeDrawHLine,
        ModeDrawVLine,
        ModeDrawAngledLine,
        ModeDrawAngle,
        ModeDrawPoint,
        ModeDrawPolyline,
    };
    void setImage(const QImage &image);
    void updatePixmap(const QPixmap &pixmap);
    void setScale(qreal scale);
    void resetView();
    void translateView(const QPoint& delta);
    void scaleView(qreal factor);
    void panFinished(); // 平移结束后的处理
    // 窗宽窗位相关接口
    void setWindowLevel(int width, int level);
    void setAutoWindowing(bool enabled);
    void applyWindowLevel();  // 应用当前窗宽窗位并更新显示
    int bitDepth() const { return m_bitDepth; }  // 获取位深
    int currentWindowWidth() const ;  // 获取当前窗宽
    int currentWindowLevel() const ;  // 获取当前窗位
    QRectF imageBounds() const;
    void setToolMode(ToolMode mode);
    ToolMode currentToolMode() const;
    void updatePixelInfo(const QPointF &scenePos);
    int getPixelValue(int x, int y) const; // 确保声明
signals:
    void scaleChanged(qreal scale);  // 原有：缩放变化信号

    // 信号，用于通知外部（MainWindow 更新 UI，如 infoWidget）
    void windowLevelChanged(int width, int level);  // 窗宽窗位变化
    void autoWindowingToggled(bool enabled);  // 自动窗宽窗位切换
    void pixelInfoChanged(int x, int y, int value);  // 鼠标位置的像素信息（坐标 + 灰度值，value 为 -1 表示 N/A）
    void viewZoomed(qreal factor);
    /**
     * @brief 当视图平移时发射。
     */
    void viewPanned();
public slots:
    void onWindowChanged(int value);  // 从 UI 滑动条接收窗宽变化
    void onLevelChanged(int value);   // 从 UI 滑动条接收窗位变化
    /**
     * @brief 槽：当一个ROI请求被提取时调用
     * @param roi 请求提取的ROI实例
     */
    void onExtractRegion(ROI* roi);
protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // 直接在 ImageViewer 中重写鼠标事件，处理绘制逻辑（取代 MainWindow 的 eventFilter）
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    qreal m_initialScale;
    QGraphicsRectItem *m_borderItem;
    void fitToView();
    // 图像数据和处理状态
    QRectF m_imageBounds;
    QImage m_originalImage;  // 原始图像
    int m_bitDepth = 8;      // 位深（8 或 16）
    int m_windowWidth = 256; // 当前窗宽（默认值）
    int m_windowLevel = 128; // 当前窗位（默认值）
    bool m_autoWindowing = false;  // 自动窗宽窗位状态
    ToolMode m_currentToolMode = ModeSelect; // 默认是选择模式

    void calculateAutoWindowLevel(int &min, int &max);           // 计算自动窗宽窗位
    DrawingStateMachine* m_drawingStateMachine;
    bool m_drawingEnabled;
};

#endif // IMAGEVIEWER_H
