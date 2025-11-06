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
#include <QRectF>

class QGraphicsRectItem;
class QGraphicsLineItem;
class AnnotationPointItem;
class DrawingStateMachine;
class ROI;
class ExtractedImageViewer;
class ImageViewModel; // 前向声明

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    QGraphicsPixmapItem *pixmapItem() const { return m_pixmapItem; }

    void setViewModel(ImageViewModel *viewModel);
    ImageViewModel* viewModel() const { return m_viewModel; }
    enum ToolMode {
        ModeSelect,
        ModeDrawRect,
        ModeDrawEllipse,
        ModeDrawLine,
        ModeDrawHLine,
        ModeDrawVLine,
        ModeDrawAngledLine,
        ModeDrawAngle,
        ModeDrawPoint,
        ModeDrawPolyline,
        ModeDrawFreehand,
        ModeDrawText,
    };

    void setScale(qreal scale);
    void resetView();
    void translateView(const QPoint& delta);
    void scaleView(qreal factor);
    void panFinished(); // 平移结束后的处理

    QRectF imageBounds() const;
    void setToolMode(ToolMode mode);
    ToolMode currentToolMode() const;

signals:
    void scaleChanged(qreal scale);

    void viewZoomed(qreal factor);
    /**
     * @brief 当视图平移时发射。
     */
    void viewPanned();

public slots:
    // 用于 ViewModel 连接
    void updatePixmap(const QPixmap &pixmap);

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
    void fitToView();
    // 内部方法，用于转发像素信息请求
    void updatePixelInfo(const QPointF &scenePos);

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    qreal m_initialScale;
    QGraphicsRectItem *m_borderItem;

    QRectF m_imageBounds; // 用于绘图边界

    ToolMode m_currentToolMode = ModeSelect;
    DrawingStateMachine* m_drawingStateMachine;
    bool m_drawingEnabled;

    // 指向 ViewModel
    ImageViewModel *m_viewModel = nullptr;
};

#endif // IMAGEVIEWER_H
