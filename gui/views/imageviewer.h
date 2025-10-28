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
// #include "gui/states/drawingstatemachine.h"


class QGraphicsRectItem;
class QGraphicsLineItem;
class AnnotationPointItem;
class DrawingStateMachine;

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void loadImage(const QString &filePath);  // 原有：加载图像
    QGraphicsPixmapItem *pixmapItem() const { return m_pixmapItem; }  // 原有

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
    // ++ 新增：绘图使能控制 ++
    bool isDrawingEnabled() const;
    void setDrawingEnabled(bool enabled);

    // ++ 新增：像素信息更新方法声明 ++
    void updatePixelInfo(const QPointF &scenePos);
    int getPixelValue(int x, int y) const; // 确保声明
signals:
    void scaleChanged(qreal scale);  // 原有：缩放变化信号

    // 信号，用于通知外部（MainWindow 更新 UI，如 infoWidget）
    void windowLevelChanged(int width, int level);  // 窗宽窗位变化
    void autoWindowingToggled(bool enabled);  // 自动窗宽窗位切换
    void pixelInfoChanged(int x, int y, int value);  // 鼠标位置的像素信息（坐标 + 灰度值，value 为 -1 表示 N/A）
    void viewZoomed(qreal factor);
public slots:
    void onWindowChanged(int value);  // 从 UI 滑动条接收窗宽变化
    void onLevelChanged(int value);   // 从 UI 滑动条接收窗位变化
protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // 直接在 ImageViewer 中重写鼠标事件，处理绘制逻辑（取代 MainWindow 的 eventFilter）
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    qreal m_initialScale;
    QGraphicsRectItem *m_borderItem;
    void fitToView();
    // 图像数据和处理状态（迁移自 MainWindow）
    QImage m_originalImage;  // 原始图像
    int m_bitDepth = 8;      // 位深（8 或 16）
    int m_windowWidth = 256; // 当前窗宽（默认值）
    int m_windowLevel = 128; // 当前窗位（默认值）
    bool m_autoWindowing = false;  // 自动窗宽窗位状态


    void calculateAutoWindowLevel(int &min, int &max);           // 计算自动窗宽窗位
    // ++ 新增：状态机指针 ++
    DrawingStateMachine* m_drawingStateMachine;
    // ++ 新增：绘图使能标志 ++
    bool m_drawingEnabled;
};

#endif // IMAGEVIEWER_H
