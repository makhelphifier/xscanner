#include "toprightinfowidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSlider>

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

    // --- Window Slider ---
    windowSlider = new QSlider(Qt::Horizontal);
    windowSlider->setRange(1, 512); // 设置一个合理的范围
    windowSlider->setValue(256);
    connect(windowSlider, &QSlider::valueChanged, this, &TopRightInfoWidget::windowChanged);

    // --- Level Slider ---
    levelSlider = new QSlider(Qt::Horizontal);
    levelSlider->setRange(0, 255);
    levelSlider->setValue(128);
    connect(levelSlider, &QSlider::valueChanged, this, &TopRightInfoWidget::levelChanged);

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

    QHBoxLayout *scaleLayout = new QHBoxLayout;
    scaleLayout->addWidget(new QLabel("scale:"));
    scaleLayout->addWidget(scaleSpinBox);

    // 添加滑动条到布局
    QFormLayout *wlLayout = new QFormLayout;
    wlLayout->addRow("W:", windowSlider);
    wlLayout->addRow("L:", levelSlider);
    mainLayout->addLayout(wlLayout);

    mainLayout->addLayout(scaleLayout);
    setLayout(mainLayout);
}


void TopRightInfoWidget::setWindowValue(int value)
{
    bool oldSignalsState = windowSlider->blockSignals(true);
    windowSlider->setValue(value);
    windowSlider->blockSignals(oldSignalsState);
}

void TopRightInfoWidget::setLevelValue(int value)
{
    bool oldSignalsState = levelSlider->blockSignals(true);
    levelSlider->setValue(value);
    levelSlider->blockSignals(oldSignalsState);
}

void TopRightInfoWidget::setWindowRange(int min, int max) {
    windowSlider->setRange(min, max);
}

void TopRightInfoWidget::setLevelRange(int min, int max) {
    levelSlider->setRange(min, max);
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
