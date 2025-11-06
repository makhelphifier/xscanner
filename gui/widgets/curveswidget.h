#ifndef CURVESWIDGET_H
#define CURVESWIDGET_H

#include "qcustomplot.h"
#include <QMap>

// 前向声明
class QCPBars;
class QCPGraph;
class QCPItemRect;

/**
 * @brief 继承自 QCustomPlot，用于显示直方图和可交互的灰度调整曲线。
 *
 * 职责：
 * 1. 显示直方图 (m_histogramBars)
 * 2. 显示窗宽窗位 (m_windowLevelIndicator)
 * 3. 显示曲线 (m_curveGraph) 和锚点 (m_anchorPointsGraph)
 * 4. 处理鼠标交互 (拖动锚点、添加/删除锚点)
 * 5. 发出信号 (request...) 通知 ViewModel 更改模型
 * 6. 接收槽 (update...) 来根据 ViewModel 的更新刷新视图
 */
class CurvesWidget : public QCustomPlot
{
    Q_OBJECT

public:
    explicit CurvesWidget(QWidget *parent = nullptr);
    virtual ~CurvesWidget() = default;

signals:
    /**
     * @brief [View -> VM] 请求在曲线上添加一个新锚点
     */
    void requestAddPoint(double key, double value);
    /**
     * @brief [View -> VM] 请求移动一个现有锚点
     */
    void requestMovePoint(double oldKey, double newKey, double newValue);
    /**
     * @brief [View -> VM] 请求删除一个锚点
     */
    void requestRemovePoint(double key);

public slots:
    /**
     * @brief [VM -> View] 接收 ViewModel 的直方图数据
     */
    void updateHistogram(const QVector<double> &data, const QCPRange &keyRange);

    /**
     * @brief [VM -> View] 接收 ViewModel 的窗宽窗位并更新指示器
     */
    void updateWindowLevelIndicator(double width, double level);

    /**
     * @brief [VM -> View] 接收 ViewModel 的曲线数据并更新
     */
    void updateCurve(const QMap<double, double> &points);

    /**
     * @brief [VM -> View] (新) 根据 VM 的模式显示/隐藏 窗宽/曲线
     * @param isWindowLevelMode true: 显示窗宽指示器; false: 显示曲线图
     */
    void setWindowLevelMode(bool isWindowLevelMode);


protected:
    // --- QCustomPlot 鼠标事件重写 ---
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    /**
     * @brief 初始化绘图控件的外观
     */
    void setupPlot();

    /**
     * @brief [辅助] 根据锚点生成平滑曲线的数据
     */
    void generateSmoothCurve(const QMap<double, double> &points,
                             QVector<double> &keys,
                             QVector<double> &values);

    /**
     * @brief [辅助] 线性插值计算
     */
    double interpolateCurve(double key, const QMap<double, double> &curvePoints);

    // --- 图层 ---
    QCPBars *m_histogramBars;         // 图层 0: 背景直方图
    QCPGraph *m_curveGraph;           // 图层 1: 平滑曲线
    QCPGraph *m_anchorPointsGraph;    // 图层 2: 可拖动锚点

    // --- 窗宽窗位 ---
    QCPItemRect *m_windowLevelIndicator; // 仅在 W/L 模式下显示

    // --- 交互状态 ---
    bool m_isDraggingPoint;     // 是否正在拖动一个锚点
    double m_draggedPointKey;   // 正在被拖动的锚点的 Key
    bool m_dragStartedOnCurve;  // 鼠标是否在曲线上按下的

    QMap<double, double> m_currentPoints; // 缓存当前锚点，用于插值
};

#endif // CURVESWIDGET_H
