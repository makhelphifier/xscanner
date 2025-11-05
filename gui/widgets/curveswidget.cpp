#include "curveswidget.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QtGlobal>

// 曲线插值的精细度
static const int CURVE_RESOLUTION = 256;

CurvesWidget::CurvesWidget(QWidget *parent)
    : QCustomPlot(parent),
    m_histogramBars(nullptr),
    m_curveGraph(nullptr),
    m_anchorPointsGraph(nullptr),
    m_windowLevelIndicator(nullptr),
    m_isDraggingPoint(false),
    m_draggedPointKey(0.0),
    m_dragStartedOnCurve(false)
{
    // 1. 创建图层 (按Z轴顺序)

    // 图层 0: 直方图 (复制自 HistogramWidget)
    m_histogramBars = new QCPBars(this->xAxis, this->yAxis);
    m_histogramBars->setLayer("background"); // 放在最底层
    m_histogramBars->setAntialiased(false);
    m_histogramBars->setBrush(QColor(150, 150, 150));
    m_histogramBars->setPen(Qt::NoPen);

    // 图层 0: 窗宽窗位指示器 (复制自 HistogramWidget)
    m_windowLevelIndicator = new QCPItemRect(this);
    m_windowLevelIndicator->setLayer("background");
    m_windowLevelIndicator->setBrush(QColor(255, 255, 255, 30));
    m_windowLevelIndicator->setPen(Qt::NoPen);

    // 图层 1: 曲线
    m_curveGraph = this->addGraph(this->xAxis, this->yAxis);
    m_curveGraph->setLayer("main");
    m_curveGraph->setPen(QPen(Qt::white, 2));

    // 图层 2: 锚点
    m_anchorPointsGraph = this->addGraph(this->xAxis, this->yAxis);
    m_anchorPointsGraph->setLayer("overlay"); // 放在最顶层
    m_anchorPointsGraph->setLineStyle(QCPGraph::lsNone);
    m_anchorPointsGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(0, 120, 215), Qt::white, 7));
    m_anchorPointsGraph->setSelectable(QCP::stSingleData); // 允许点选

    // 2. 初始化绘图样式
    setupPlot();

    // 3. 初始状态
    setWindowLevelMode(true); // 默认显示窗宽窗位
}

/**
 * @brief [新] Catmull-Rom 样条插值辅助函数
 * @param p0 控制点 P0 (t=-1)
 * @param p1 控制点 P1 (t=0)
 * @param p2 控制点 P2 (t=1)
 * @param p3 控制点 P3 (t=2)
 * @param t 归一化的插值因子 (0.0 到 1.0)，代表在 P1 和 P2 之间的位置
 * @return 插值后的值
 */
static inline double catmullRomInterpolate(double p0, double p1, double p2, double p3, double t)
{
    double t2 = t * t;
    double t3 = t2 * t;

    return 0.5 * (
               (2.0 * p1) +
               (-p0 + p2) * t +
               (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2 +
               (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3
               );
}

/**
 * @brief (复制自 HistogramWidget) 初始化绘图控件的外观
 */
void CurvesWidget::setupPlot()
{
    // 绘图区域样式 (暗黑)
    this->setBackground(QColor(39, 39, 44));
    this->xAxis->setBasePen(QPen(Qt::white));
    this->yAxis->setBasePen(QPen(Qt::white));
    this->xAxis->setTickPen(QPen(Qt::white));
    this->yAxis->setTickPen(QPen(Qt::white));
    this->xAxis->setSubTickPen(QPen(Qt::white));
    this->yAxis->setSubTickPen(QPen(Qt::white));
    this->xAxis->setTickLabelColor(Qt::white);
    this->yAxis->setTickLabelColor(Qt::white);

    // Y轴使用对数刻度 (用于直方图)
    this->yAxis->setScaleType(QCPAxis::stLogarithmic);

    // [修改] Y轴2 (yAxis2) 用于曲线，使用线性刻度 [0-255]
    this->yAxis2->setVisible(true);
    this->yAxis2->setBasePen(QPen(Qt::white));
    this->yAxis2->setTickPen(QPen(Qt::white));
    this->yAxis2->setSubTickPen(QPen(Qt::white));
    this->yAxis2->setTickLabelColor(Qt::white);
    this->yAxis2->setScaleType(QCPAxis::stLinear);
    this->yAxis2->setRange(0, 255); // 固定输出范围

    // 将曲线和锚点关联到 yAxis2
    m_curveGraph->setValueAxis(this->yAxis2);
    m_anchorPointsGraph->setValueAxis(this->yAxis2);

    // 启用拖动和缩放
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

/**
 * @brief [VM -> View] (复制自 HistogramWidget) 更新直方图
 */
void CurvesWidget::updateHistogram(const QVector<double> &data, const QCPRange &keyRange)
{
    int numBins = data.size();
    if (numBins == 0) {
        m_histogramBars->setData(QVector<double>(), QVector<double>());
        this->replot();
        return;
    }

    // 1. 计算 X 轴的 keys
    QVector<double> keys(numBins);
    double binWidth = keyRange.size() / (double)numBins;
    double halfBinWidth = binWidth / 2.0;

    for (int i = 0; i < numBins; ++i) {
        keys[i] = keyRange.lower + (i * binWidth) + halfBinWidth;
    }

    // 2. 设置柱状图数据和宽度
    m_histogramBars->setWidth(binWidth);
    m_histogramBars->setData(keys, data);

    // 3. 设置 X 轴范围
    this->xAxis->setRange(keyRange);

    // 4. 自动缩放 Y 轴 (对数轴)
    this->yAxis->rescale();

    // 5. 更新指示器的 Y 轴范围 (使用 yAxis)
    double currentTopLeftKey = m_windowLevelIndicator->topLeft->key();
    double currentBottomRightKey = m_windowLevelIndicator->bottomRight->key();

    m_windowLevelIndicator->topLeft->setCoords(currentTopLeftKey, this->yAxis->range().upper);
    m_windowLevelIndicator->bottomRight->setCoords(currentBottomRightKey, this->yAxis->range().lower);

    // 6. 重绘
    this->replot();
}

/**
 * @brief [VM -> View] (复制自 HistogramWidget) 更新窗宽窗位指示器
 */
void CurvesWidget::updateWindowLevelIndicator(double width, double level)
{
    // 1. 计算范围
    double min = level - width / 2.0;
    double max = level + width / 2.0;

    // 2. 更新指示器的 X 轴范围 (Y 轴保持在 yAxis 的范围)
    double currentTopLeftValue = m_windowLevelIndicator->topLeft->value();
    double currentBottomRightValue = m_windowLevelIndicator->bottomRight->value();

    // 如果 Y 轴范围无效 (例如刚启动)，则使用默认值
    if (qFuzzyCompare(currentTopLeftValue, currentBottomRightValue)) {
        currentTopLeftValue = this->yAxis->range().upper;
        currentBottomRightValue = this->yAxis->range().lower;
    }

    m_windowLevelIndicator->topLeft->setCoords(min, currentTopLeftValue);
    m_windowLevelIndicator->bottomRight->setCoords(max, currentBottomRightValue);

    // 3. 重绘
    this->replot();
}

/**
 * @brief [VM -> View] (新) 接收 ViewModel 的曲线数据并更新
 */
void CurvesWidget::updateCurve(const QMap<double, double> &points)
{
    m_currentPoints = points;

    // 1. 更新锚点 (m_anchorPointsGraph)
    QVector<double> keys, values;
    for (auto it = points.constBegin(); it != points.constEnd(); ++it) {
        keys.append(it.key());
        values.append(it.value());
    }
    m_anchorPointsGraph->setData(keys, values);

    // 2. 更新平滑曲线 (m_curveGraph)
    QVector<double> smoothKeys, smoothValues;
    generateSmoothCurve(points, smoothKeys, smoothValues);
    m_curveGraph->setData(smoothKeys, smoothValues);

    // 3. 重绘
    this->replot();
}

/**
 * @brief [VM -> View] (新) 根据 VM 的模式显示/隐藏 窗宽/曲线
 */
void CurvesWidget::setWindowLevelMode(bool isWindowLevelMode)
{
    m_windowLevelIndicator->setVisible(isWindowLevelMode);
    m_curveGraph->setVisible(!isWindowLevelMode);
    m_anchorPointsGraph->setVisible(!isWindowLevelMode);

    // [修复] QCPAxis 没有 setInteraction。
    // 我们必须通过 AxisRect 来控制 Y 轴的拖动/缩放。
    if (isWindowLevelMode) {
        // W/L 模式: 禁用 Y 轴拖动/缩放 (保留 X 轴设置)
        this->axisRect()->setRangeDrag(this->axisRect()->rangeDrag() & ~Qt::Vertical);
        this->axisRect()->setRangeZoom(this->axisRect()->rangeZoom() & ~Qt::Vertical);
    } else {
        // 曲线模式: 启用 Y 轴拖动/缩放
        this->axisRect()->setRangeDrag(this->axisRect()->rangeDrag() | Qt::Vertical);
        this->axisRect()->setRangeZoom(this->axisRect()->rangeZoom() | Qt::Vertical);
        // 并且明确指定只有 yAxis (直方图) 可以拖动/缩放, yAxis2 (曲线) 不行
        this->axisRect()->setRangeDragAxes(this->axisRect()->rangeDragAxes(Qt::Horizontal), {this->yAxis});
        this->axisRect()->setRangeZoomAxes(this->axisRect()->rangeZoomAxes(Qt::Horizontal), {this->yAxis});
    }
    if (isWindowLevelMode) {
        // W/L 模式下，Y 轴自动缩放
        this->yAxis->rescale();
    }

    this->replot();
}


// --- 鼠标交互 ---

void CurvesWidget::mousePressEvent(QMouseEvent *event)
{
    m_isDraggingPoint = false;
    m_dragStartedOnCurve = false;

    // 如果是 W/L 模式，则执行 QCustomPlot 默认交互 (例如缩放/拖动)
    if (m_windowLevelIndicator->visible()) {
        QCustomPlot::mousePressEvent(event);
        return;
    }

    // --- 曲线模式下的交互 ---

    // 1. 检查是否点中了锚点
    // (使用 yAxis2，因为锚点图层在该轴上)
    QVariant selectionDetails;
    double anchorDistance = m_anchorPointsGraph->selectTest(event->pos(), false, &selectionDetails);
    // QCPDataSelection selection = m_anchorPointsGraph->selectTest(event->pos(), false);
    QCPDataSelection selection;
    if (anchorDistance < this->selectionTolerance()) {
        selection = selectionDetails.value<QCPDataSelection>();
    }
    if (event->button() == Qt::LeftButton) {
        if (selection.dataPointCount() > 0) {
            // --- A. 点中了锚点：准备拖动 ---
            m_isDraggingPoint = true;
            // 获取被选中点的数据
            int index = selection.dataRange(0).begin();
            m_draggedPointKey = m_anchorPointsGraph->data()->at(index)->key;
            this->setCursor(Qt::ClosedHandCursor);
        } else {
            // --- B. 未点中锚点：检查是否点中了曲线 ---
            // (使用 yAxis2)
            double dist = m_curveGraph->selectTest(event->pos(), false);
            if (dist >= 0 && dist < 8) { // 8 像素的容差
                m_dragStartedOnCurve = true;
                this->setCursor(Qt::PointingHandCursor);
            } else {
                m_dragStartedOnCurve = false;
                this->setCursor(Qt::OpenHandCursor);
            }
        }
    } else if (event->button() == Qt::RightButton) {
        // --- C. 右键点击：删除锚点 ---
        if (selection.dataPointCount() > 0) {
            int index = selection.dataRange(0).begin();
            double keyToRemove = m_anchorPointsGraph->data()->at(index)->key;
            emit requestRemovePoint(keyToRemove);
        }
    }

    // 阻止 QCustomPlot 的默认拖动（例如轴拖动）
    // event->accept(); // 不调用基类
}

void CurvesWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 如果是 W/L 模式，则执行 QCustomPlot 默认交互
    if (m_windowLevelIndicator->visible()) {
        QCustomPlot::mouseMoveEvent(event);
        return;
    }

    // --- 曲线模式下的交互 ---
    if (m_isDraggingPoint) {
        // --- A. 正在拖动锚点 ---
        double newKey = this->xAxis->pixelToCoord(event->pos().x());
        double newValue = this->yAxis2->pixelToCoord(event->pos().y());

        // 钳制
        newKey = qBound(xAxis->range().lower, newKey, xAxis->range().upper);
        newValue = qBound(0.0, newValue, 255.0);

        // 发出信号
        emit requestMovePoint(m_draggedPointKey, newKey, newValue);

        // 更新拖动键 (关键)
        m_draggedPointKey = newKey;
    }
    else {
        // --- B. 未拖动锚点，执行 QCustomPlot 默认交互 (轴拖动/缩放) ---
        QCustomPlot::mouseMoveEvent(event);
    }
}

void CurvesWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果是 W/L 模式，则执行 QCustomPlot 默认交互
    if (m_windowLevelIndicator->visible()) {
        QCustomPlot::mouseReleaseEvent(event);
        return;
    }

    // --- 曲线模式下的交互 ---

    if (m_isDraggingPoint) {
        // --- A. 停止拖动锚点 ---
        m_isDraggingPoint = false;
    }
    else if (m_dragStartedOnCurve && event->button() == Qt::LeftButton) {
        // --- B. 在曲线上释放：添加锚点 ---
        double newKey = this->xAxis->pixelToCoord(event->pos().x());
        double newValue = this->yAxis2->pixelToCoord(event->pos().y());

        // 钳制
        newKey = qBound(xAxis->range().lower, newKey, xAxis->range().upper);
        newValue = qBound(0.0, newValue, 255.0);

        emit requestAddPoint(newKey, newValue);
    }
    else {
        // --- C. 其他情况，执行 QCustomPlot 默认交互 ---
        QCustomPlot::mouseReleaseEvent(event);
    }

    m_dragStartedOnCurve = false;
    this->setCursor(Qt::ArrowCursor); // 恢复光标
}


// --- 辅助函数 ---

/**
 * @brief [辅助] 根据锚点生成平滑曲线的数据
 */
void CurvesWidget::generateSmoothCurve(const QMap<double, double> &points,
                                       QVector<double> &keys,
                                       QVector<double> &values)
{
    if (points.size() < 2)
        return;

    keys.resize(CURVE_RESOLUTION);
    values.resize(CURVE_RESOLUTION);

    double minKey = points.firstKey();
    double maxKey = points.lastKey();
    double keyRange = maxKey - minKey;

    if (keyRange <= 0) // 范围无效
        return;

    for (int i = 0; i < CURVE_RESOLUTION; ++i) {
        double key = minKey + (keyRange * i) / (CURVE_RESOLUTION - 1);
        double value = interpolateCurve(key, points);
        keys[i] = key;
        values[i] = value;
    }
}

/**
 * @brief [辅助] 线性插值计算 (逻辑同 ImageProcessor)
 */
double CurvesWidget::interpolateCurve(double key, const QMap<double, double> &curvePoints)
{
    // 1. 安全检查
    if (curvePoints.size() < 2) {
        return 0.0;
    }

    // 2. 找到 key 所在的分段 (P1 -> P2)
    auto it_p2 = curvePoints.lowerBound(key);

    // 3. 处理边界情况 (key 超出范围)
    if (it_p2 == curvePoints.constEnd()) {
        // 大于最后一个点
        return curvePoints.last();
    }
    if (it_p2 == curvePoints.constBegin()) {
        // 小于等于第一个点
        return it_p2.value();
    }

    // 4. 找到所有4个控制点 (P0, P1, P2, P3)
    auto it_p1 = it_p2 - 1;

    // 5. 处理曲线端点（创建“幽灵点” P0 和 P3）
    auto it_p0 = (it_p1 == curvePoints.constBegin()) ? it_p1 : (it_p1 - 1);
    auto it_p3 = (it_p2 + 1 == curvePoints.constEnd()) ? it_p2 : (it_p2 + 1);

    // 6. 提取 Y 值 (p0_y, p1_y, p2_y, p3_y)
    double p0_y = it_p0.value();
    double p1_y = it_p1.value();
    double p2_y = it_p2.value();
    double p3_y = it_p3.value();

    // 7. 特殊处理：如果我们在第一个/最后一个真实分段，
    //    我们需要修改 P0/P3 的Y值来使曲线端点平滑
    if (it_p1 == curvePoints.constBegin()) {
        // 我们在第一个分段 (P1 -> P2)。
        // 强制 P0=P2，使 P1 处的切线为 0。
        p0_y = p2_y;
    }
    if (it_p2 + 1 == curvePoints.constEnd()) {
        // 我们在最后一个分段 (P(n-1) -> Pn)。
        // 强制 P(n+1) = P(n-1)，使 Pn 处的切线为 0。
        p3_y = p1_y;
    }

    // 8. 计算 t (key 在 P1 和 P2 之间的归一化位置 0.0-1.0)
    double t = 0.0;
    double denom = (it_p2.key() - it_p1.key());
    if (denom > 1e-9) { // 避免除以零
        t = (key - it_p1.key()) / denom;
    }

    // 9. 调用样条插值 (返回 double，由调用者 qBound)
    return catmullRomInterpolate(p0_y, p1_y, p2_y, p3_y, t);
}
