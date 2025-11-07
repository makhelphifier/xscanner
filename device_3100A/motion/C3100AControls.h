#ifndef C3100ACUSTOMCONTROL_H
#define C3100ACUSTOMCONTROL_H

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QValidator>

/**
 * @brief 封装了轴ID的 QDoubleSpinBox
 */
class C3100ADoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    explicit C3100ADoubleSpinBox(int axisId = -1, QWidget *parent = nullptr)
        : QDoubleSpinBox(parent), m_axisId(axisId)
    {
        // setButtonSymbols(QAbstractSpinBox::NoButtons);
        setDecimals(4);
        setRange(-99999.0, 99999.0);
        lineEdit()->setValidator(new QDoubleValidator(-99999.0, 99999.0, 4, this));
        setFont(QFont("Microsoft YaHei", 9));
    }

    void setAxisId(int id) { m_axisId = id; }
    int axisId() const { return m_axisId; }

private:
    int m_axisId;
};

/**
 * @brief 封装了轴ID的 QPushButton
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
 * @brief  QSpinBox (用于电压/电流)
 */
class C3100AIntSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit C3100AIntSpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        // setButtonSymbols(QAbstractSpinBox::NoButtons);
        setRange(0, 9999);
        setFont(QFont("Microsoft YaHei", 9));
    }
};


#endif
