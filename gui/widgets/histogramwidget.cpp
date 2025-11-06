#include "histogramwidget.h"
#include "qcustomplot.h"
#include <QVBoxLayout>
#include <QDebug>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent)
{
    // 1. 创建 QCustomPlot 实例
    m_plot = new QCustomPlot(this);

    // 2. 创建 Bars (柱状图)
    // 构造函数(通过 QCPAxis)会自动将其注册到 m_plot
    m_histogramBars = new QCPBars(m_plot->xAxis, m_plot->yAxis);

    // 3. 创建窗宽窗位指示器 (矩形)
    // 构造函数(通过 QCustomPlot* parent)会自动将其注册到 m_plot
    m_windowLevelIndicator = new QCPItemRect(m_plot);

    // 4. 设置布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_plot);
    this->setLayout(layout);

    // 5. 初始化绘图样式
    setupPlot();
}

/**
 * @brief (新) 初始化绘图控件的外观
 */
void HistogramWidget::setupPlot()
{
    // 柱状图样式
    m_histogramBars->setAntialiased(false);
    m_histogramBars->setBrush(QColor(150, 150, 150));
    m_histogramBars->setPen(Qt::NoPen);

    // 窗宽窗位指示器样式
    m_windowLevelIndicator->setBrush(QColor(255, 255, 255, 30)); // 半透明白色
    m_windowLevelIndicator->setPen(Qt::NoPen);

    // 绘图区域样式 (暗黑)
    m_plot->setBackground(QColor(39, 39, 44));
    m_plot->xAxis->setBasePen(QPen(Qt::white));
    m_plot->yAxis->setBasePen(QPen(Qt::white));
    m_plot->xAxis->setTickPen(QPen(Qt::white));
    m_plot->yAxis->setTickPen(QPen(Qt::white));
    m_plot->xAxis->setSubTickPen(QPen(Qt::white));
    m_plot->yAxis->setSubTickPen(QPen(Qt::white));
    m_plot->xAxis->setTickLabelColor(Qt::white);
    m_plot->yAxis->setTickLabelColor(Qt::white);

    // Y轴使用对数刻度
    m_plot->yAxis->setScaleType(QCPAxis::stLogarithmic);

    // 启用拖动和缩放
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

/**
 * @brief (新) 槽：更新直方图数据
 */
void HistogramWidget::updateHistogram(const QVector<double> &data, const QCPRange &keyRange)
{
    int numBins = data.size();
    if (numBins == 0) {
        m_histogramBars->setData(QVector<double>(), QVector<double>());
        m_plot->replot();
        return;
    }

    // 1. 计算 X 轴的 keys (每个柱子的中心点)
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
    m_plot->xAxis->setRange(keyRange);

    // 4. 自动缩放 Y 轴
    m_plot->yAxis->rescale();

    // 5. (修复 API) 更新指示器的 Y 轴范围 (使用 setCoords)
    // (保留 X 轴位置，更新 Y 轴位置)
    double currentTopLeftKey = m_windowLevelIndicator->topLeft->key();
    double currentBottomRightKey = m_windowLevelIndicator->bottomRight->key();

    m_windowLevelIndicator->topLeft->setCoords(currentTopLeftKey, m_plot->yAxis->range().upper);
    m_windowLevelIndicator->bottomRight->setCoords(currentBottomRightKey, m_plot->yAxis->range().lower);


    // 6. 重绘
    m_plot->replot();
}

/**
 * @brief (新) 槽：更新窗宽窗位指示器
 */
void HistogramWidget::updateWindowLevelIndicator(double width, double level)
{
    // 1. 计算范围
    double min = level - width / 2.0;
    double max = level + width / 2.0;

    // 2. (修复 API) 更新指示器的 X 轴范围 (使用 setCoords)
    // (保留 Y 轴位置，更新 X 轴位置)
    double currentTopLeftValue = m_windowLevelIndicator->topLeft->value();
    double currentBottomRightValue = m_windowLevelIndicator->bottomRight->value();

    m_windowLevelIndicator->topLeft->setCoords(min, currentTopLeftValue);
    m_windowLevelIndicator->bottomRight->setCoords(max, currentBottomRightValue);

    // 3. 重绘
    m_plot->replot();
}
