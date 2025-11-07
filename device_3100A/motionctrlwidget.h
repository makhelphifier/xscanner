#ifndef MOTIONCTRLWIDGET_H
#define MOTIONCTRLWIDGET_H

#include "qmessagebox.h"
#include <QWidget>
#include "cardio.h"
#include "appconfig.h"
#include "axisformula.h"
#include "axisparametersdialog.h"

namespace Ui {
class MotionCtrlWidget;
}

class QTimer;
class QMessageBox;
class MotionCtrlViewModel;
class ParaSettingsDialog;
class OtherControlsDialog;

class MotionCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MotionCtrlWidget(QWidget *parent = nullptr);
    ~MotionCtrlWidget();


    void setStartAxis(bool val);
private slots:

    void on_btn_test_clicked();

    void on_btn_IO_2_clicked();
    void on_PosLimit();
    void on_lineEdit_returnPressed();

    void slot_DataChanged(const QVariant &var);
private:

    void connections();

    void showMessageBox();

    Ui::MotionCtrlWidget *ui;

    MotionCtrlViewModel *mcViewModel;

    ParaSettingsDialog *paraSettingsDialog;
    OtherControlsDialog *otherControlsDialog;
    cardIO *cardIO_ = nullptr;

    AxisFormula *axisFormula_ = nullptr;

    QMessageBox msgBox;

    AppConfig* appcinfig = nullptr;


    MotionCtrlManager *mcManger = nullptr;
    AxisParametersDialog *axisParamsDialog;

};

#endif // MOTIONCTRLWIDGET_H
