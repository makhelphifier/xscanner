// gui/toprightinfowidget.cpp

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
    windowLevelLabel = new QLabel("window/level: 120.5/231");

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

    mainLayout->addLayout(scaleLayout);

    setLayout(mainLayout);
}

void TopRightInfoWidget::setScale(qreal scale)
{
    // 阻止信号循环：在以编程方式设置值时暂时断开连接
    bool oldSignalsState = scaleSpinBox->blockSignals(true);
    scaleSpinBox->setValue(scale);
    scaleSpinBox->blockSignals(oldSignalsState);
}
