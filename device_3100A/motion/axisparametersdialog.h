#ifndef AXISPARAMETERSDIALOG_H
#define AXISPARAMETERSDIALOG_H

#include <QWidget>

// Forward declarations
class QLineEdit;

class AxisParametersDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AxisParametersDialog(QWidget *parent = nullptr);
    ~AxisParametersDialog();

private slots:
    void on_applyButton_clicked();

private:
    void loadParameters();

    // 射线源Z轴
    QLineEdit *xrayZ_Coeff_lineEdit;
    QLineEdit *xrayZ_Offset_lineEdit;

    // 探测器Z1轴
    QLineEdit *detectorZ1_Coeff_lineEdit;
    QLineEdit *detectorZ1_Offset_lineEdit;

    // 探测器R轴
    QLineEdit *detectorR_Coeff_lineEdit;
    QLineEdit *detectorR_Offset_lineEdit;

    // 探测器theta轴
    QLineEdit *detectorTheta_Coeff_lineEdit;
    QLineEdit *detectorTheta_Offset_lineEdit;

    // 载物台X轴
    QLineEdit *tableX_Coeff_lineEdit;
    QLineEdit *tableX_Offset_lineEdit;

    // 载物台Y轴
    QLineEdit *tableY_Coeff_lineEdit;
    QLineEdit *tableY_Offset_lineEdit;
};

#endif // AXISPARAMETERSDIALOG_H
