#ifndef IMAGEGRAPHICSVIEW_H
#define IMAGEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QPolygonF>
#include <QStyleOptionGraphicsItem>
#include <QScrollBar>
#include <QMenu>
#include <QColorDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <vector>
#include <cmath>


// 窗位窗宽计算结果结构体
struct WindowLevelResult {
    double windowLevel;  // 窗位（亮度）
    double windowWidth;  // 窗宽（对比度）
    bool isValid;        // 计算是否成功

    WindowLevelResult() : windowLevel(0), windowWidth(0), isValid(false) {}
    WindowLevelResult(double level, double width) : windowLevel(level), windowWidth(width), isValid(true) {}
};

// 窗位窗宽计算工作类
class WindowLevelCalculator : public QObject
{
    Q_OBJECT

public:
    struct CalculationParams {
        QImage image;
        QRectF region;

        CalculationParams() = default;
        CalculationParams(const QImage& img, const QRectF& rect) : image(img), region(rect) {}
    };

    static WindowLevelResult calculateOptimalWindowLevel(const CalculationParams& params);
    static QImage applyWindowLevel(const QImage& image, double windowLevel, double windowWidth);

private:
    static std::vector<int> extractPixelValues(const QImage& image, const QRectF& region);
    static WindowLevelResult calculateHistogramBasedWindowLevel(const std::vector<int>& pixelValues, int autoThreshold);
    static double calculateMean(const std::vector<int>& values);
    static double calculateStandardDeviation(const std::vector<int>& values, double mean);
    static double calculatePercentile(std::vector<int> values, double percentile);
};

// 自定义图形项基类
class AnnotationItem : public QGraphicsItem
{
public:
    enum AnnotationType {
        Line,
        Rectangle,
        Ellipse,
        Arrow,
        Text,
        Measurement
    };

    AnnotationItem(AnnotationType type, QGraphicsItem* parent = nullptr);
    virtual ~AnnotationItem() = default;

    AnnotationType getType() const { return m_type; }
    void setPen(const QPen& pen) { m_pen = pen; }
    QPen getPen() const { return m_pen; }
    void setBrush(const QBrush& brush) { m_brush = brush; }
    QBrush getBrush() const { return m_brush; }

    virtual void setStartPoint(const QPointF& point) { m_startPoint = point; }
    virtual void setEndPoint(const QPointF& point) { m_endPoint = point; }
    QPointF getStartPoint() const { return m_startPoint; }
    QPointF getEndPoint() const { return m_endPoint; }

protected:
    AnnotationType m_type;
    QPen m_pen;
    QBrush m_brush;
    QPointF m_startPoint;
    QPointF m_endPoint;
};

// 线条标注
class LineAnnotation : public AnnotationItem
{
public:
    LineAnnotation(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

// 矩形标注
class RectangleAnnotation : public AnnotationItem
{
public:
    RectangleAnnotation(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

// 椭圆标注
class EllipseAnnotation : public AnnotationItem
{
public:
    EllipseAnnotation(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

// 箭头标注
class ArrowAnnotation : public AnnotationItem
{
public:
    ArrowAnnotation(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QPolygonF createArrowHead(const QPointF& start, const QPointF& end) const;
};

// 文本标注
class TextAnnotation : public AnnotationItem
{
public:
    TextAnnotation(const QString& text = "", QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    void setText(const QString& text) { m_text = text; }
    QString getText() const { return m_text; }
    void setFont(const QFont& font) { m_font = font; }
    QFont getFont() const { return m_font; }

private:
    QString m_text;
    QFont m_font;
};

// 测量标注
class MeasurementAnnotation : public AnnotationItem
{
public:
    MeasurementAnnotation(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    void setPixelScale(double scale) { m_pixelScale = scale; }
    double getPixelScale() const { return m_pixelScale; }
    double getDistance() const;
    QString getDistanceText() const;

private:
    double m_pixelScale; // 像素到实际单位的比例
    QString m_unit;
};

// ROI 蒙层控件
class ROIOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ROIOverlayWidget(QWidget* parent = nullptr);

    void setROIRect(const QRect& rect);
    void clearROI();
    bool isDrawing() const { return m_isDrawing; }
    void setDrawing(bool drawing) { m_isDrawing = drawing; }

    // 图像信息显示功能
    void setImageInfo(const QString& info);
    void setMouseInfo(const QString& info);
    void setImageInfoPosition(const QPoint& pos);
    void setMouseInfoPosition(const QPoint& pos);

signals:
    void roiFinished(const QRect& rect);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QRect m_roiRect;
    bool m_isDrawing;

    // 信息显示相关
    QString m_imageInfo;
    QString m_mouseInfo;
    QPoint m_imageInfoPos;
    QPoint m_mouseInfoPos;
    QFont m_infoFont;

    void drawInfoText(QPainter& painter, const QString& text, const QPoint& pos);
};

// 自定义图形视图
class ImageGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    enum InteractionMode {
        Pan,
        Annotate,
        Measure
    };

    enum AnnotationTool {
        LineTool,
        RectangleTool,
        EllipseTool,
        ArrowTool,
        TextTool,
        MeasurementTool
    };

    explicit ImageGraphicsView(QWidget* parent = nullptr);
    ~ImageGraphicsView();

    void setInteractionMode(InteractionMode mode);
    InteractionMode getInteractionMode() const { return m_interactionMode; }

    void setAnnotationTool(AnnotationTool tool);
    AnnotationTool getAnnotationTool() const { return m_annotationTool; }

    void setAnnotationPen(const QPen& pen) { m_annotationPen = pen; }
    QPen getAnnotationPen() const { return m_annotationPen; }

    void setAnnotationBrush(const QBrush& brush) { m_annotationBrush = brush; }
    QBrush getAnnotationBrush() const { return m_annotationBrush; }

    void clearAnnotations();
    QList<AnnotationItem*> getAnnotations() const { return m_annotations; }

    // Image management methods
    void loadImage(const QPixmap& pixmap);
    void loadImageImmediate(const QPixmap& pixmap);
    void loadImageHighFrequency(const QPixmap& pixmap); // 高频率图像加载专用
    void clearImage(); // 清除图像但保持pixmap item // 立即加载图像，不使用延迟
    void setCurrentImage(const QPixmap& pixmap);
    QPixmap getCurrentPixmap() const { return m_currentPixmap; }
    QGraphicsScene* getGraphicsScene() const { return m_graphicsScene; }
    QGraphicsPixmapItem* getPixmapItem() const { return m_pixmapItem; }

    // 窗位窗宽计算和调整方法
    void calculateAndApplyOptimalWindowLevel(const QRectF& region);

    // 仅计算最佳窗位窗宽（不自动应用）
    // void calculateOptimalWindowLevel(const QRectF& region);
    // QImage autoAdjust(const QImage& image, int autoThreshold, int& ww, int& wl);

    // 手动应用指定的窗位窗宽到图像
    void applyWindowLevel(double windowCenter, double windowWidth);

signals:
    void scaleChanged(double scale);
    void annotationAdded(AnnotationItem* item);
    void measurementTaken(double distance, const QString& unit);
    void rectangleSelected(const QRectF& rect); // 矩形选择信号，坐标为图像原始尺寸
    void windowLevelCalculationStarted(); // 窗位窗宽计算开始信号
    void windowLevelCalculationFinished(const WindowLevelResult& result); // 窗位窗宽计算完成信号
    void windowLevelApplied(double windowLevel, double windowWidth); // 窗位窗宽应用完成信号

    // 内部信号，用于跨线程图像加载
    void imageLoadRequested(const QPixmap& pixmap);
    void imageLoadHighFrequencyRequested(const QPixmap& pixmap);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void showContextMenu(const QPoint& pos);
    void deleteSelectedAnnotation();
    void editAnnotationProperties();
    void onWindowLevelCalculationFinished();
    void onWindowLevelApplicationFinished();
    // 延迟图像加载处理已移除，现在使用直接加载

    // 跨线程图像加载槽函数
    void onImageLoadRequested(const QPixmap& pixmap);
    void onImageLoadHighFrequencyRequested(const QPixmap& pixmap);

private:
    void scaleView(double scaleFactor);
    void startAnnotation(const QPointF& scenePos);
    void updateAnnotation(const QPointF& scenePos);
    void finishAnnotation(const QPointF& scenePos);
    AnnotationItem* createAnnotationItem();
    void updateCursor();
    void updateInfoLabels();
    void updateMousePosition(const QPoint& viewPos);
    void loadImageInternal(const QPixmap& pixmap); // 内部图像加载方法
    void initializePixmapItem(); // 初始化pixmap item，确保始终存在

    // 右键矩形选择相关方法
    bool isPointInImage(const QPointF& scenePos) const;
    void startROIDrawing(const QPoint& viewPos);
    void updateROIDrawing(const QPoint& viewPos);
    void finishROIDrawing();

private:
    // Graphics scene and pixmap management
    QGraphicsScene* m_graphicsScene;
    QGraphicsPixmapItem* m_pixmapItem;

    InteractionMode m_interactionMode;
    AnnotationTool m_annotationTool;
    QPen m_annotationPen;
    QBrush m_annotationBrush;

    bool m_isDrawing;
    AnnotationItem* m_currentAnnotation;
    QList<AnnotationItem*> m_annotations;

    QPoint m_lastPanPoint;
    bool m_isPanning;

    double m_scaleFactor;
    static const double MIN_SCALE;
    static const double MAX_SCALE;

    // 空格键状态和光标管理
    bool m_spacePressed;
    Qt::CursorShape m_originalCursor;

    // 图像信息显示
    QPixmap m_originPixmap;
    QPixmap m_currentPixmap;
    QPoint m_lastMousePos;

    // 右键ROI绘制
    bool m_isRightDragging;        // 是否正在右键拖拽
    QPoint m_rightDragStart;       // 右键拖拽起始点（视图坐标）
    QPoint m_rightDragCurrent;     // 右键拖拽当前点（视图坐标）
    QPoint m_rightClickStart;      // 右键点击起始位置（视图坐标）
    ROIOverlayWidget* m_roiOverlay; // ROI蒙层控件

    // 窗位窗宽计算相关
    QFutureWatcher<WindowLevelResult>* m_windowLevelCalculationWatcher;
    QFutureWatcher<QImage>* m_windowLevelApplicationWatcher;
    QRectF m_currentCalculationRegion; // 当前计算区域
    WindowLevelResult m_lastCalculationResult; // 最后一次计算结果
    bool m_autoApplyAfterCalculation; // 计算完成后是否自动应用

    // 频繁图像加载处理
    QPixmap m_pendingPixmap;           // 待加载的图像
    bool m_hasPendingImage;            // 是否有待加载的图像
    QMutex m_imageLoadMutex;           // 图像加载互斥锁
    bool m_isLoadingImage;             // 是否正在加载图像
};

#endif // IMAGEGRAPHICSVIEW_H
