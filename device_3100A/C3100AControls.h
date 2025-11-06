#ifndef C3100ACUSTOMCONTROL_H
#define C3100ACUSTOMCONTROL_H

#include <QDoubleSpinBox>
#include <QSpinBox> // <--- 新增
#include <QPushButton>
#include <QLineEdit>
#include <QValidator>
#include "device_3100A/C3100AAxisDef.h" // 包含我们的轴定义

/**
 * @brief 封装了轴ID的 QDoubleSpinBox (3100A专用)
 * 命名为 C3100ADoubleSpinBox 以避免和 3600 项目的 GyDoubleSpinBox 冲突
 */
class C3100ADoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    explicit C3100ADoubleSpinBox(int axisId = -1, QWidget *parent = nullptr)
        : QDoubleSpinBox(parent), m_axisId(axisId)
    {
        // 模仿老UI (motionctrlwidget.cpp) 和 3600 UI 的风格
        // setButtonSymbols(QAbstractSpinBox::NoButtons);
        setDecimals(2); // 老UI使用4位小数
        setRange(-99999.0, 99999.0); // 设置一个合理的默认范围

        // 确保只能输入浮点数
        lineEdit()->setValidator(new QDoubleValidator(-99999.0, 99999.0, 4, this));

        // 设置默认字体（可选，保持风格统一）
        setFont(QFont("Microsoft YaHei", 9));
    }

    void setAxisId(int id) { m_axisId = id; }
    int axisId() const { return m_axisId; }

private:
    int m_axisId;
};

/**
 * @brief 封装了轴ID的 QPushButton (3100A专用)
 * 命名为 C3100APushButton 以避免和 3600 项目的 GyPushButton 冲突
 */
class C3100APushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit C3100APushButton(int axisId = -1, const QString &text = "", QWidget *parent = nullptr)
        : QPushButton(text, parent), m_axisId(axisId)
    {
        setFont(QFont("Microsoft YaHei", 9));
        setMinimumWidth(40); // 给予按钮一个合理的最小宽度
    }

    void setAxisId(int id) { m_axisId = id; }
    int axisId() const { return m_axisId; }

private:
    int m_axisId;
};

/**
 * @brief 3100A专用的 QSpinBox (用于电压/电流)
 * 模仿同事的 GyIntSpinBox
 */
class C3100AIntSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit C3100AIntSpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        // 模仿 3600 UI 的风格
        // setButtonSymbols(QAbstractSpinBox::NoButtons);
        setRange(0, 9999); // 设置一个合理的默认范围
        setFont(QFont("Microsoft YaHei", 9));
    }
};


#endif // C3100ACUSTOMCONTROL_H
