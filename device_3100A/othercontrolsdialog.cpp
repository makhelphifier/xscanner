#include "othercontrolsdialog.h"
#include "ui_othercontrolsdialog.h"

OtherControlsDialog::OtherControlsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OtherControlsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    enableStatus = QVector<bool>(15,true);
    connections();
}

OtherControlsDialog::~OtherControlsDialog()
{
    delete ui;
}

void OtherControlsDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        if (event->pos().y() < 30) {
            m_lMousePress = true;
            m_pos = event->pos();
        }
    }
}

void OtherControlsDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_lMousePress) {
        this->move(this->mapToGlobal(event->pos())-m_pos);
    }
}

void OtherControlsDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_lMousePress = false;
}

void OtherControlsDialog::connections()
{
    //========================👇回零操作=================================
    connect(ui->btn_resetPos_1,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(0);//探测源 Z 轴
    });
    connect(ui->btn_resetPos_2,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(1);//光管 Z 轴
    });
    connect(ui->btn_resetPos_3,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(2);//探测源 X 轴
    });
    connect(ui->btn_resetPos_4,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(3);//载物台X轴
    });
    connect(ui->btn_resetPos_5,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(4);//探测源 Y 轴
    });
    connect(ui->btn_resetPos_6,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(5);//载物台Y轴
    });
    connect(ui->btn_resetPos_7,&QPushButton::clicked,this,[=]()->void{
        emit sig_resetPos(6);//旋转轴
    });

    //========================👆回零操作=================================

    //========================👇使能=================================
    connect(ui->btn_enable_1,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(0,!enableStatus[0]);//探测源 Z 轴
    });
    connect(ui->btn_enable_2,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(1,!enableStatus[1]);//光管 Z 轴
    });
    connect(ui->btn_enable_3,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(2,!enableStatus[2]);//探测源 X 轴
    });
    connect(ui->btn_enable_4,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(3,!enableStatus[3]);//载物台 X 轴
    });
    connect(ui->btn_enable_5,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(4,!enableStatus[4]);//探测源 Y 轴
    });
    connect(ui->btn_enable_6,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(5,!enableStatus[5]);//载物台 Y 轴
    });
    connect(ui->btn_enable_7,&QPushButton::clicked,this,[=]()->void{
        emit sig_enable(6,!enableStatus[6]);//旋转轴
    });

    //========================👆使能=================================

    //========================👇轴停止=================================
    connect(ui->btn_stop_1,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(0);
    });
    connect(ui->btn_stop_2,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(1);
    });
    connect(ui->btn_stop_3,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(2);
    });
    connect(ui->btn_stop_4,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(3);
    });
    connect(ui->btn_stop_5,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(4);
    });
    connect(ui->btn_stop_6,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(5);
    });
    connect(ui->btn_stop_7,&QPushButton::clicked,this,[=]()->void{
        emit sig_stop(6);
    });

    //==========================👆轴停止===============================
}


void OtherControlsDialog::on_btn_close_clicked()
{
    this->close();
}

void OtherControlsDialog::on_enableChanged(int index, bool status)
{
    if(status == enableStatus[index]){
        return;
    }
    QString btnName;
    if(status) {
        btnName = u8"断开使能";
    } else{
        btnName = u8"使能";
    }
    enableStatus[index] = status;
    switch (index) {
    case 0:
        ui->btn_enable_1->setText(btnName);
        break;
    case 1:
        ui->btn_enable_2->setText(btnName);
            break;
    case 2:
        ui->btn_enable_3->setText(btnName);
            break;
    case 3:
        ui->btn_enable_4->setText(btnName);
        break;
    case 4:
        ui->btn_enable_5->setText(btnName);
        break;
    case 5:
        ui->btn_enable_6->setText(btnName);
        break;
    case 7:
        ui->btn_enable_7->setText(btnName);
        break;
    default:
        break;
    }
}

