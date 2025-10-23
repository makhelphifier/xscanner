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
#include <QMouseEvent>      // 用于鼠标事件重写
#include <QImage>           // 用于图像数据
#include <QGraphicsRectItem> // 用于预览矩形
#include <QGraphicsLineItem> // 用于预览线条
#include <QGraphicsEllipseItem>

class QGraphicsRectItem;
class QGraphicsLineItem;
class AnnotationPointItem;

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void loadImage(const QString &filePath);  // 原有：加载图像
    QGraphicsPixmapItem *pixmapItem() const { return m_pixmapItem; }  // 原有

    // 原有公共接口（保留，不重复）
    void setImage(const QImage &image);
    void updatePixmap(const QPixmap &pixmap);
    void setScale(qreal scale);
    void resetView();

    // 窗宽窗位相关接口（公共方法，便于 MainWindow 调用）
    void setWindowLevel(int width, int level);
    void setAutoWindowing(bool enabled);
    void applyWindowLevel();  // 应用当前窗宽窗位并更新显示
    int bitDepth() const { return m_bitDepth; }  // 获取位深
    int currentWindowWidth() const ;  // 获取当前窗宽
    int currentWindowLevel() const ;  // 获取当前窗位

    // 绘制模式接口
    enum DrawMode {
        Mode_Select,      // 选择/拖动模式
        Mode_Line,        // 直线绘制
        Mode_Rect,        // 矩形绘制
        Mode_Ellipse,     // 椭圆绘制
        Mode_Point,       // 点绘制
        Mode_WindowLevel,  // 窗宽窗位矩形选择
        Mode_HorizontalLine, //水平线绘制
        Mode_VerticalLine,   //垂直线绘制
    };
    void setDrawMode(DrawMode mode);  // 设置当前绘制模式
    DrawMode currentMode() const { return m_currentMode; }

Q_SIGNALS:
    void scaleChanged(qreal scale);  // 原有：缩放变化信号

    // 信号，用于通知外部（MainWindow 更新 UI，如 infoWidget）
    void windowLevelChanged(int width, int level);  // 窗宽窗位变化
    void autoWindowingToggled(bool enabled);  // 自动窗宽窗位切换
    void pixelInfoChanged(int x, int y, int value);  // 鼠标位置的像素信息（坐标 + 灰度值，value 为 -1 表示 N/A）

public slots:
    void onWindowChanged(int value);  // 从 UI 滑动条接收窗宽变化
    void onLevelChanged(int value);   // 从 UI 滑动条接收窗位变化
    void onScaleChanged(qreal scale);  // 响应 scale 变化，更新点项大小
    void clearAllAnnotations();
protected:
    // 原有：事件重写
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // 直接在 ImageViewer 中重写鼠标事件，处理绘制逻辑（取代 MainWindow 的 eventFilter）
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // 原有私有成员
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    qreal m_initialScale;
    QGraphicsRectItem *m_borderItem;
    void fitToView();  // 原有私有方法

    // 图像数据和处理状态（迁移自 MainWindow）
    QImage m_originalImage;  // 原始图像
    int m_bitDepth = 8;      // 位深（8 或 16）
    int m_windowWidth = 256; // 当前窗宽（默认值）
    int m_windowLevel = 128; // 当前窗位（默认值）
    bool m_autoWindowing = false;  // 自动窗宽窗位状态

    // 绘制状态（迁移自 MainWindow）
    DrawMode m_currentMode = Mode_Select;
    bool m_isDrawing = false;
    QPointF m_startPoint;  // 绘制起点
    QGraphicsLineItem *m_previewLine = nullptr;  // 线条预览
    QGraphicsRectItem *m_previewRect = nullptr;  // 矩形/窗宽预览

    void updatePixelInfo(const QPointF &scenePos);  // 更新像素信息（发射信号）
    int getPixelValue(int x, int y) const;          // 获取像素灰度值
    QGraphicsLineItem* createPreviewLine(const QPointF &start);  // 创建线预览
    QGraphicsRectItem* createPreviewRect(const QPointF &start);  // 创建矩形预览
    void finishDrawingLine(const QPointF &endPoint);             // 完成线绘制
    void finishWindowLevelRect(const QRectF &rect);              // 完成窗宽矩形
    void switchToSelectMode();                                   // 切换回选择模式
    void calculateAutoWindowLevel(int &min, int &max);           // 计算自动窗宽窗位
    QGraphicsEllipseItem *m_previewEllipse = nullptr;
    void finishDrawingEllipse(const QRectF &ellipseRect);  // 完成椭圆绘制
    void finishDrawingPoint(const QPointF &pointPos);  // 完成点测量
    QGraphicsEllipseItem* createPreviewEllipse(const QPointF &start);  // 创建椭圆预览
    QList<AnnotationPointItem*> m_pointItems;  // 跟踪所有点项，便于遍历更新（避免全场景遍历）
    void finishDrawingPoint(const QPointF &pointPos, qreal currentScale, int imageWidth);

};

#endif // IMAGEVIEWER_H
