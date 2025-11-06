// gui/widgets/toprightinfowidget.cpp

#include "toprightinfowidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFormLayout>

TopRightInfoWidget::TopRightInfoWidget(QWidget *parent) : QWidget(parent)
{
    // --- 设置整体样式 ---
    this->setStyleSheet(
        "background-color: rgba(39, 39, 44, 200);" // 半透明背景
        "color: white;"
        "border-radius: 4px;"
        "padding: 5px;"
        );

    autoWindowingCheckBox = new QCheckBox("自动窗宽窗位");
    connect(autoWindowingCheckBox, &QCheckBox::toggled, this, &TopRightInfoWidget::autoWindowingToggled);

    windowLevelLabel = new QLabel("window/level: 120.5/231");

    // --- Window SpinBox ---
    windowSpinBox = new QDoubleSpinBox;
    windowSpinBox->setDecimals(2);
    windowSpinBox->setRange(0.0, 65535.0);
    windowSpinBox->setSingleStep(100.0);
    windowSpinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    connect(windowSpinBox, &QDoubleSpinBox::editingFinished, this, [this]() {
        // 手动发出我们自己的信号，使用 spinBox 的当前值
        emit windowChanged(windowSpinBox->value());
    });

    // --- Level SpinBox ---
    levelSpinBox = new QDoubleSpinBox;
    levelSpinBox->setDecimals(2);
    levelSpinBox->setRange(0.0, 65535.0);
    levelSpinBox->setSingleStep(100.0);
    levelSpinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    connect(levelSpinBox, &QDoubleSpinBox::editingFinished, this, [this]() {
        // 手动发出我们自己的信号，使用 spinBox 的当前值
        emit levelChanged(levelSpinBox->value());
    });

    // --- Scale SpinBox ---
    scaleSpinBox = new QDoubleSpinBox;
    scaleSpinBox->setSuffix("X");
    scaleSpinBox->setDecimals(2);
    scaleSpinBox->setSingleStep(0.1);
    scaleSpinBox->setRange(0.1, 10.0);
    scaleSpinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    scaleSpinBox->setStyleSheet(
        "background-color: #1E1E1E;"
        "border: 1px solid gray;"
        "padding: 2px;"
        );

    // 连接信号，当用户手动编辑SpinBox时，发射我们自己的信号
    connect(scaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TopRightInfoWidget::scaleEdited);

    // --- 布局 ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    mainLayout->addWidget(autoWindowingCheckBox);
    mainLayout->addWidget(windowLevelLabel);

    // 添加 SpinBox 到布局
    QFormLayout *wlLayout = new QFormLayout;
    wlLayout->addRow("W:", windowSpinBox);
    wlLayout->addRow("L:", levelSpinBox);
    mainLayout->addLayout(wlLayout);

    QHBoxLayout *scaleLayout = new QHBoxLayout;
    scaleLayout->addWidget(new QLabel("scale:"));
    scaleLayout->addWidget(scaleSpinBox);

    mainLayout->addLayout(scaleLayout);
    setLayout(mainLayout);
}

void TopRightInfoWidget::setWindowValue(double value)
{
    bool oldSignalsState = windowSpinBox->blockSignals(true);
    windowSpinBox->setValue(value);
    windowSpinBox->blockSignals(oldSignalsState);
}

void TopRightInfoWidget::setLevelValue(double value)
{
    bool oldSignalsState = levelSpinBox->blockSignals(true);
    levelSpinBox->setValue(value);
    levelSpinBox->blockSignals(oldSignalsState);
}

void TopRightInfoWidget::setWindowRange(double min, double max)
{
    windowSpinBox->setRange(min, max);
}

void TopRightInfoWidget::setLevelRange(double min, double max)
{
    levelSpinBox->setRange(min, max);
}

void TopRightInfoWidget::setScale(qreal scale)
{
    // 阻止信号循环：在以编程方式设置值时暂时断开连接
    bool oldSignalsState = scaleSpinBox->blockSignals(true);
    scaleSpinBox->setValue(scale);
    scaleSpinBox->blockSignals(oldSignalsState);
}

void TopRightInfoWidget::setWindowLevelText(const QString &text)
{
    windowLevelLabel->setText(text);
}

void TopRightInfoWidget::uncheckAutoWindowing()
{
    // 阻止信号循环
    bool oldSignalsState = autoWindowingCheckBox->blockSignals(true);
    autoWindowingCheckBox->setChecked(false);
    autoWindowingCheckBox->blockSignals(oldSignalsState);
}

void TopRightInfoWidget::setAutoWindowingChecked(bool checked)
{
    // 阻止信号循环：在以编程方式设置值时暂时断开连接
    bool oldSignalsState = autoWindowingCheckBox->blockSignals(true);
    autoWindowingCheckBox->setChecked(checked);
    autoWindowingCheckBox->blockSignals(oldSignalsState);

    // 如果勾选，立即触发 toggled 信号以应用自动窗位窗宽
    //     if (checked) {
    //         emit autoWindowingToggled(true);
    //     }
}

void TopRightInfoWidget::checkAutoWindowing()
{
    // 阻止信号循环
    bool oldSignalsState = autoWindowingCheckBox->blockSignals(true);
    autoWindowingCheckBox->setChecked(true);
    autoWindowingCheckBox->blockSignals(oldSignalsState);
}
