/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *m_PWMON;
    QPushButton *m_PWMOFF;
    QPushButton *m_PWMCTL;
    QPushButton *m_OUTON;
    QPushButton *m_OUTOFF;
    QPushButton *m_ADREAD;
    QPushButton *m_ADSTOP;
    QPushButton *m_SAVEDDTA;
    QLCDNumber *lcdNum_ain0;
    QLCDNumber *lcdNum_ain1;
    QLCDNumber *lcdNum_ain2;
    QLCDNumber *lcdNum_ain3;
    QLCDNumber *lcdNum_ain4;
    QLCDNumber *lcdNum_ain5;
    QLCDNumber *lcdNum_ain6;
    QLCDNumber *lcdNum_ain7;
    QPushButton *m_CAN0Send;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(919, 504);
        pushButton = new QPushButton(Widget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(30, 30, 181, 91));
        pushButton_2 = new QPushButton(Widget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(230, 30, 181, 91));
        m_PWMON = new QPushButton(Widget);
        m_PWMON->setObjectName(QString::fromUtf8("m_PWMON"));
        m_PWMON->setGeometry(QRect(30, 130, 181, 91));
        m_PWMOFF = new QPushButton(Widget);
        m_PWMOFF->setObjectName(QString::fromUtf8("m_PWMOFF"));
        m_PWMOFF->setGeometry(QRect(230, 130, 181, 91));
        m_PWMCTL = new QPushButton(Widget);
        m_PWMCTL->setObjectName(QString::fromUtf8("m_PWMCTL"));
        m_PWMCTL->setGeometry(QRect(430, 130, 181, 91));
        m_OUTON = new QPushButton(Widget);
        m_OUTON->setObjectName(QString::fromUtf8("m_OUTON"));
        m_OUTON->setGeometry(QRect(30, 230, 181, 91));
        m_OUTOFF = new QPushButton(Widget);
        m_OUTOFF->setObjectName(QString::fromUtf8("m_OUTOFF"));
        m_OUTOFF->setGeometry(QRect(230, 230, 181, 91));
        m_ADREAD = new QPushButton(Widget);
        m_ADREAD->setObjectName(QString::fromUtf8("m_ADREAD"));
        m_ADREAD->setGeometry(QRect(230, 340, 181, 91));
        m_ADSTOP = new QPushButton(Widget);
        m_ADSTOP->setObjectName(QString::fromUtf8("m_ADSTOP"));
        m_ADSTOP->setGeometry(QRect(430, 340, 181, 91));
        m_SAVEDDTA = new QPushButton(Widget);
        m_SAVEDDTA->setObjectName(QString::fromUtf8("m_SAVEDDTA"));
        m_SAVEDDTA->setGeometry(QRect(430, 230, 181, 91));
        lcdNum_ain0 = new QLCDNumber(Widget);
        lcdNum_ain0->setObjectName(QString::fromUtf8("lcdNum_ain0"));
        lcdNum_ain0->setGeometry(QRect(630, 20, 111, 61));
        lcdNum_ain1 = new QLCDNumber(Widget);
        lcdNum_ain1->setObjectName(QString::fromUtf8("lcdNum_ain1"));
        lcdNum_ain1->setGeometry(QRect(630, 90, 111, 61));
        lcdNum_ain2 = new QLCDNumber(Widget);
        lcdNum_ain2->setObjectName(QString::fromUtf8("lcdNum_ain2"));
        lcdNum_ain2->setGeometry(QRect(630, 160, 111, 61));
        lcdNum_ain3 = new QLCDNumber(Widget);
        lcdNum_ain3->setObjectName(QString::fromUtf8("lcdNum_ain3"));
        lcdNum_ain3->setGeometry(QRect(630, 230, 111, 61));
        lcdNum_ain4 = new QLCDNumber(Widget);
        lcdNum_ain4->setObjectName(QString::fromUtf8("lcdNum_ain4"));
        lcdNum_ain4->setGeometry(QRect(760, 20, 111, 61));
        lcdNum_ain5 = new QLCDNumber(Widget);
        lcdNum_ain5->setObjectName(QString::fromUtf8("lcdNum_ain5"));
        lcdNum_ain5->setGeometry(QRect(760, 90, 111, 61));
        lcdNum_ain6 = new QLCDNumber(Widget);
        lcdNum_ain6->setObjectName(QString::fromUtf8("lcdNum_ain6"));
        lcdNum_ain6->setGeometry(QRect(760, 160, 111, 61));
        lcdNum_ain7 = new QLCDNumber(Widget);
        lcdNum_ain7->setObjectName(QString::fromUtf8("lcdNum_ain7"));
        lcdNum_ain7->setGeometry(QRect(760, 230, 111, 61));
        m_CAN0Send = new QPushButton(Widget);
        m_CAN0Send->setObjectName(QString::fromUtf8("m_CAN0Send"));
        m_CAN0Send->setGeometry(QRect(30, 340, 181, 91));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("Widget", "OPEN", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("Widget", "CLOSE", 0, QApplication::UnicodeUTF8));
        m_PWMON->setText(QApplication::translate("Widget", "PWM_ON", 0, QApplication::UnicodeUTF8));
        m_PWMOFF->setText(QApplication::translate("Widget", "PWM_OFF", 0, QApplication::UnicodeUTF8));
        m_PWMCTL->setText(QApplication::translate("Widget", "PWM_CTL", 0, QApplication::UnicodeUTF8));
        m_OUTON->setText(QApplication::translate("Widget", "OUT_ON", 0, QApplication::UnicodeUTF8));
        m_OUTOFF->setText(QApplication::translate("Widget", "OUT_OFF", 0, QApplication::UnicodeUTF8));
        m_ADREAD->setText(QApplication::translate("Widget", "AD_READ", 0, QApplication::UnicodeUTF8));
        m_ADSTOP->setText(QApplication::translate("Widget", "AD_STOP", 0, QApplication::UnicodeUTF8));
        m_SAVEDDTA->setText(QApplication::translate("Widget", "SAVEDATA", 0, QApplication::UnicodeUTF8));
        m_CAN0Send->setText(QApplication::translate("Widget", "CAN0_Send", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
