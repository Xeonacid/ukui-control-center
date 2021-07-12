﻿ /* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "projection.h"
#include "ui_projection.h"

#include <QProcess>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>
#include <QMouseEvent>

#define ITEMFIXEDHEIGH 58
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"

enum {
    NOT_SUPPORT_P2P = 0,
    SUPPORT_P2P_WITHOUT_DEV,
    SUPPORT_P2P_PERFECT,
    OP_NO_RESPONSE
};

Projection::Projection()
{
    pluginName = tr("Projection");
    //~ contents_path /bluetooth/Bluetooth
    pluginType = DEVICES;
    ui = new Ui::Projection;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);
    projectionBtn = new SwitchButton(pluginWidget);
    int result = system("checkDaemonRunning.sh");
    if (result != 0) {
        projectionBtn->setChecked(true);
    }
//   ui->pinframe->hide();

    connect(projectionBtn, SIGNAL(checkedChanged(bool)), this, SLOT(projectionButtonClickSlots(bool)));
    // m_pin = new QLabel(pluginWidget);
    ui->label->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
    //~ contents_path /bluetooth/Open Bluetooth
    ui->titleLabel->setText(tr("Open Projection"));
    ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->projectionName->setStyleSheet("QLineEdit{background-color:transparent;"
                                              "border-width:0;");
    m_pServiceInterface = new QDBusInterface("org.freedesktop.miracleagent",
                                             "/org/freedesktop/miracleagent",
                                             "org.freedesktop.miracleagent.op",
                                             QDBusConnection::sessionBus());
    QString path=QDir::homePath()+"/.config/miracast.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    bool bo=setting->contains("host");
    qDebug()<<bo<<"bo";
    if (!bo) {
        QDBusInterface *hostInterface = new QDBusInterface("org.freedesktop.hostname1",
                                                          "/org/freedesktop/hostname1",
                                                          "org.freedesktop.hostname1",
                                                          QDBusConnection::systemBus());
        hostName = hostInterface->property("Hostname").value<QString>();
        setting->setValue("host",hostName);
        setting->sync();
        setting->endGroup();
        initComponent();
    }else {
        hostName = setting->value("host").toString();
    }
    ui->projectionName->setText(hostName);

    ui->projectionNameChange->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ui->projectionNameChange->size()));

    // ui->pronamelabel->setMaxLength(16);
    // ui->pronamelabel->setAlignment(Qt::AlignRight);
    ui->projectionNameWidget->installEventFilter(this);
    ui->horizontalLayout->addWidget(projectionBtn);
    initComponent();
}

void Projection::changeProjectionName(QString name){
    qDebug() << name;
    QString path=QDir::homePath()+"/.config/miracast.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    setting->setValue("host",name);
    setting->sync();
    setting->endGroup();
    m_pServiceInterface->call("UiSetName",name);
    ui->projectionName->setText(name);
}

void Projection::showChangeProjectionNameDialog(){

    ChangeProjectionName * dialog = new ChangeProjectionName();

    connect(dialog, &ChangeProjectionName::sendNewProjectionName, [=](QString name){
        changeProjectionName(name);
    });
    dialog->exec();
}

bool Projection::getWifiStatus() {
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );
    // 获取当前wifi是否打开
    QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");
    if (m_result.isValid()) {
        bool status = m_result.value().toBool();
        return status;
    } else {
        qDebug()<<"org.freedesktop.NetworkManager get invalid"<<endl;
        return false;
    }
}

void Projection::setWifiStatus(bool status) {

    QString wifiStatus = status ? "on" : "off";
    QString program = "nmcli";
    QStringList arg;
    arg << "radio" << "wifi" << wifiStatus;
    QProcess *nmcliCmd = new QProcess(this);
    nmcliCmd->start(program, arg);
    nmcliCmd->waitForStarted();
}

bool Projection::eventFilter(QObject *watched, QEvent *event){

    if (watched == ui->projectionNameWidget){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                showChangeProjectionNameDialog();
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void Projection::catchsignal()
{
    while (1)
    {
        m_pServiceInterface = new QDBusInterface("org.freedesktop.miracle.wifi",
                                                 "/org/freedesktop/miracle/wifi/ui",
                                                 "org.freedesktop.miracle.wifi.ui",
                                                 QDBusConnection::systemBus());
        if (m_pServiceInterface->isValid()) {
            connect(m_pServiceInterface,SIGNAL(PinCode(QString, QString)),this,SLOT(projectionPinSlots(QString,QString)));
            return;
        }else {
            qDebug()<<"失败";
            delete m_pServiceInterface;
            delaymsec(1000);
        }
    }
\
}
void Projection::delaymsec(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

Projection::~Projection()
{
    delete ui;
    delete m_pServiceInterface;
}

QString Projection::get_plugin_name(){

    return pluginName;
}

int Projection::get_plugin_type(){
    return pluginType;
}

QWidget *Projection::get_plugin_ui(){
    QDBusMessage result = m_pServiceInterface->call("PreCheck");
    QList<QVariant> outArgs = result.arguments();
    int projectionstatus = outArgs.at(0).value<int>();
    qDebug() << "---->" << projectionstatus;
    ui->widget->hide();
    ui->label->hide();
    ui->label_3->hide();
    ui->widget_2->show();
    ui->label_setsize->setText("");
     projectionstatus=1;//
    if (NOT_SUPPORT_P2P == projectionstatus) {
        ui->label_2->setText("未检测到无线网卡或网卡驱动不支持，投屏功能不可用");
        ui->projectionNameWidget->setEnabled(false);
        projectionBtn->setEnabled(false);
    }

    else if (SUPPORT_P2P_WITHOUT_DEV == projectionstatus) {
        if(getWifiStatus())
        {
            qDebug()<<"wifi is on now";
            ui->label_3->setText("使用时请保持WLAN处于开启状态，投屏过程中可能会中断无线连接");
            ui->widget->show();
            ui->label->show();
            ui->label_3->show();
            ui->widget_2->hide();
        }
        else
        {
            qDebug()<<"wifi is off now";
            ui->label_2->setText("WLAN未开启，请打开WLAN开关");
        }

        projectionBtn->setEnabled(true);
    }
    else if (OP_NO_RESPONSE == projectionstatus) {

        ui->label_2->setText("该功能软件服务异常，请稍后再试");
        ui->projectionNameWidget->setEnabled(true);
        projectionBtn->setEnabled(true);
    }

    return pluginWidget;
}

void Projection::plugin_delay_control(){

}

const QString Projection::name() const {

    return QStringLiteral("projection");
}

void Projection::projectionPinSlots(QString type, QString pin) {
    if (type.contains("clear")) {
        //m_pin->clear();
    } else {
        qDebug()<<pin;
        //m_pin->setText(pin);
    }
}

void Projection::projectionButtonClickSlots(bool status) {

    if (status){        
         m_pServiceInterface->call("Start",ui->projectionName->text(),"");
    } else {
        m_pServiceInterface->call("Stop");
    }
}

void Projection::initComponent(){

    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
    addWgt->setMinimumSize(QSize(580, 64));
    addWgt->setMaximumSize(QSize(16777215, 64));
    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(base); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #2FB3E8; border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    //~ contents_path /bluetooth/Add Bluetooths
    QLabel * textLabel = new QLabel(tr("Add Bluetooths"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addItem(new QSpacerItem(8,10,QSizePolicy::Fixed));
    addLyt->addWidget(iconLabel);
    addLyt->addItem(new QSpacerItem(16,10,QSizePolicy::Fixed));
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addWgt->setLayout(addLyt);
    addWgt->hide();
}