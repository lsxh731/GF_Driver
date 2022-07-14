#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#define UBUNTU

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    char *buzzer_ctl = "/dev/buzzer_ctl";
    fd_beep = 0;

#ifdef UBUNTU
    if((fd_beep = open(buzzer_ctl,O_RDWR|O_NOCTTY|O_NDELAY))<0){
        qDebug()<<"##### open %s failed! ######";
    }
    ioctl(fd_beep,0,0);
#endif
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
#ifdef UBUNTU
    ioctl(fd_beep,1,0);
#endif
    qDebug()<<"##### beep open successed! ######";
}

void Widget::on_pushButton_2_clicked()
{
#ifdef UBUNTU
    ioctl(fd_beep,0,0);
#endif
    qDebug()<<"##### beep close successed! ######";
}
