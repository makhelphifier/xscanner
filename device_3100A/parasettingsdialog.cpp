#include "parasettingsdialog.h"
#include "ui_parasettingsdialog.h"

ParaSettingsDialog::ParaSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParaSettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
}

ParaSettingsDialog::~ParaSettingsDialog()
{
    delete ui;
}

void ParaSettingsDialog::resetWindow(int index)
{
    curIndex = index;
    switch (index) {
    case 0:
        ui->title->setText(u8"轴1参数设置");
        break;
    case 1:
        ui->title->setText(u8"轴2参数设置");
        break;
    case 2:
        ui->title->setText(u8"轴3参数设置");
        break;
    case 3:
        ui->title->setText(u8"轴4参数设置");
        break;
    case 4:
        ui->title->setText(u8"轴5参数设置");
        break;
    case 5:
        ui->title->setText(u8"轴6参数设置");
        break;
    case 6:
        ui->title->setText(u8"轴7参数设置");
        break;
    case 7:
        ui->title->setText(u8"轴8参数设置");
        break;
    case 8:
        ui->title->setText(u8"轴9参数设置");
        break;
    case 9:
        ui->title->setText(u8"轴10参数设置");
        break;
    case 10:
        ui->title->setText(u8"轴11参数设置");
        break;
    case 11:
        ui->title->setText(u8"轴12参数设置");
        break;
    case 12:
        ui->title->setText(u8"轴13参数设置");
        break;
    default:
        break;
    }
}

void ParaSettingsDialog::setDdSpeed(QString value)
{
    ui->lineEdit->setText(value);
}

void ParaSettingsDialog::setAbsSpeed(QString value)
{
    ui->lineEdit_2->setText(value);
}

void ParaSettingsDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        if (event->pos().y() < 30) {
            m_lMousePress = true;
            m_pos = event->pos();
        }
    }
}

void ParaSettingsDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_lMousePress) {
        this->move(this->mapToGlobal(event->pos())-m_pos);
    }
}

void ParaSettingsDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_lMousePress = false;
}

void ParaSettingsDialog::on_btn_close_clicked()
{
    this->close();
}


void ParaSettingsDialog::on_pushButton_clicked()
{
    this->close();
    emit sig_saveSpeed(curIndex,ui->lineEdit->text(),ui->lineEdit_2->text());
}


void ParaSettingsDialog::on_pushButton_2_clicked()
{
    this->close();
}

