#include "axisparametersdialog.h"
#include "appconfig.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

AxisParametersDialog::AxisParametersDialog(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("轴参数设置");
    QGridLayout *mainLayout = new QGridLayout(this);

    // 表头
    mainLayout->addWidget(new QLabel("轴名称"), 0, 0);
    mainLayout->addWidget(new QLabel("系数"), 0, 1);
    mainLayout->addWidget(new QLabel("补偿"), 0, 2);

    // 轴名称
    QStringList axisNames = {
        "射线源Z轴", "探测器Z1轴", "探测器R轴",
        "探测器theta轴", "载物台X轴", "载物台Y轴"
    };

    // --- 为每个轴创建控件 ---

    // 1. 射线源Z轴
    mainLayout->addWidget(new QLabel(axisNames[0]), 1, 0);
    xrayZ_Coeff_lineEdit = new QLineEdit();
    xrayZ_Offset_lineEdit = new QLineEdit();
    mainLayout->addWidget(xrayZ_Coeff_lineEdit, 1, 1);
    mainLayout->addWidget(xrayZ_Offset_lineEdit, 1, 2);

    // 2. 探测器Z1轴
    mainLayout->addWidget(new QLabel(axisNames[1]), 2, 0);
    detectorZ1_Coeff_lineEdit = new QLineEdit();
    detectorZ1_Offset_lineEdit = new QLineEdit();
    mainLayout->addWidget(detectorZ1_Coeff_lineEdit, 2, 1);
    mainLayout->addWidget(detectorZ1_Offset_lineEdit, 2, 2);

    // 3. 探测器R轴
    mainLayout->addWidget(new QLabel(axisNames[2]), 3, 0);
    detectorR_Coeff_lineEdit = new QLineEdit();
    detectorR_Offset_lineEdit = new QLineEdit();
    mainLayout->addWidget(detectorR_Coeff_lineEdit, 3, 1);
    mainLayout->addWidget(detectorR_Offset_lineEdit, 3, 2);

    // 4. 探测器theta轴
    mainLayout->addWidget(new QLabel(axisNames[3]), 4, 0);
    detectorTheta_Coeff_lineEdit = new QLineEdit();
    detectorTheta_Offset_lineEdit = new QLineEdit();
    mainLayout->addWidget(detectorTheta_Coeff_lineEdit, 4, 1);
    mainLayout->addWidget(detectorTheta_Offset_lineEdit, 4, 2);

    // 5. 载物台X轴
    mainLayout->addWidget(new QLabel(axisNames[4]), 5, 0);
    tableX_Coeff_lineEdit = new QLineEdit();
    tableX_Offset_lineEdit = new QLineEdit();
    mainLayout->addWidget(tableX_Coeff_lineEdit, 5, 1);
    mainLayout->addWidget(tableX_Offset_lineEdit, 5, 2);

    // 6. 载物台Y轴
    mainLayout->addWidget(new QLabel(axisNames[5]), 6, 0);
    tableY_Coeff_lineEdit = new QLineEdit();
    tableY_Offset_lineEdit = new QLineEdit();
    mainLayout->addWidget(tableY_Offset_lineEdit, 6, 2);
    mainLayout->addWidget(tableY_Coeff_lineEdit, 6, 1);


    // "应用" 按钮
    QPushButton *applyButton = new QPushButton("应用");
    mainLayout->addWidget(applyButton, 7, 0, 1, 3); // 跨3列

    // 加载初始值
    loadParameters();

    // 连接信号
    connect(applyButton, &QPushButton::clicked, this, &AxisParametersDialog::on_applyButton_clicked);
}

AxisParametersDialog::~AxisParametersDialog() {}

void AxisParametersDialog::loadParameters()
{
    // 从AppConfig加载值并设置到输入框
    AppConfig config;
    config.readConfig(); // 确保加载最新的配置

    xrayZ_Coeff_lineEdit->setText(QString::number(AppConfig::xrayZCoefficient, 'f', 4));
    xrayZ_Offset_lineEdit->setText(QString::number(AppConfig::xrayZOffset, 'f', 4));

    detectorZ1_Coeff_lineEdit->setText(QString::number(AppConfig::detectorZCoefficient, 'f', 4));
    detectorZ1_Offset_lineEdit->setText(QString::number(AppConfig::detectorZOffset, 'f', 4));

    detectorR_Coeff_lineEdit->setText(QString::number(AppConfig::detectorRCoefficient, 'f', 4));
    detectorR_Offset_lineEdit->setText(QString::number(AppConfig::detectorROffset, 'f', 4));

    detectorTheta_Coeff_lineEdit->setText(QString::number(AppConfig::detectorThetaCoefficient, 'f', 4));
    detectorTheta_Offset_lineEdit->setText(QString::number(AppConfig::detectorThetaOffset, 'f', 4));

    tableX_Coeff_lineEdit->setText(QString::number(AppConfig::tableXCoefficient, 'f', 4));
    tableX_Offset_lineEdit->setText(QString::number(AppConfig::tableXOffset, 'f', 4));

    tableY_Coeff_lineEdit->setText(QString::number(AppConfig::tableYCoefficient, 'f', 4));
    tableY_Offset_lineEdit->setText(QString::number(AppConfig::tableYOffset, 'f', 4));
}

void AxisParametersDialog::on_applyButton_clicked()
{
    // 将输入框的值保存回AppConfig
    AppConfig::xrayZCoefficient = xrayZ_Coeff_lineEdit->text().toFloat();
    AppConfig::xrayZOffset = xrayZ_Offset_lineEdit->text().toFloat();

    AppConfig::detectorZCoefficient = detectorZ1_Coeff_lineEdit->text().toFloat();
    AppConfig::detectorZOffset = detectorZ1_Offset_lineEdit->text().toFloat();

    AppConfig::detectorRCoefficient = detectorR_Coeff_lineEdit->text().toFloat();
    AppConfig::detectorROffset = detectorR_Offset_lineEdit->text().toFloat();

    AppConfig::detectorThetaCoefficient = detectorTheta_Coeff_lineEdit->text().toFloat();
    AppConfig::detectorThetaOffset = detectorTheta_Offset_lineEdit->text().toFloat();

    AppConfig::tableXCoefficient = tableX_Coeff_lineEdit->text().toFloat();
    AppConfig::tableXOffset = tableX_Offset_lineEdit->text().toFloat();

    AppConfig::tableYCoefficient = tableY_Coeff_lineEdit->text().toFloat();
    AppConfig::tableYOffset = tableY_Offset_lineEdit->text().toFloat();

    // 写入配置文件
    AppConfig config;
    config.writeConfig();

    QMessageBox::information(this, "成功", "参数保存成功！");
    this->close();
}
