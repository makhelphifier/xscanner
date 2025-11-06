#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QVector>
#include "qcustomplot.h" // 需要 QCPRange (值传递) 和基类

// 前向声明
class QCPBars;
class QCPItemRect; // <-- 1. 添加 QCPItemRect 前向声明

/**
 * @brief (新) 用于显示直方图的 QCustomPlot 封装控件
 */
class HistogramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget *parent = nullptr);
    virtual ~HistogramWidget() = default;

public slots:
    /**
     * @brief (新) 接收 ViewModel 的数据并更新直方图
     * @param data 像素计数值 (Y 轴)
     * @param keyRange 像素强度范围 (X 轴)
     */
    void updateHistogram(const QVector<double> &data, const QCPRange &keyRange);

    /**
     * @brief (新) 接收 ViewModel 的窗宽窗位并更新指示器
     * @param width 当前窗宽
     * @param level 当前窗位
     */
    void updateWindowLevelIndicator(double width, double level); // <-- 2. 添加新槽

private:
    /**
     * @brief (新) 用于初始化绘图控件的外观
     */
    void setupPlot();

    QCustomPlot *m_plot;
    QCPBars *m_histogramBars;
    QCPItemRect *m_windowLevelIndicator; // <-- 3. 添加指示器成员
};

#endif // HISTOGRAMWIDGET_H
