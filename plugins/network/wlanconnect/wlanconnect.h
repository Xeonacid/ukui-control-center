/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#ifndef WLANCONNECT_H
#define WLANCONNECT_H

#include <QObject>
#include <QtPlugin>

#include <QNetworkInterface>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QtNetwork>

#include <QTimer>
#include <QStringList>
#include <QString>
#include <QGSettings>
#include <QListWidget>
#include <QListWidgetItem>
#include <HoverWidget/hoverwidget.h>
#include <QMap>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "shell/interface.h"
#include "SwitchButton/switchbutton.h"
#include "commonComponent/HoverBtn/hoverbtn.h"
#include "itemframe.h"
#include "wlanitem.h"
namespace Ui {
class WlanConnect;
}


class WlanConnect : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    WlanConnect();
    ~WlanConnect();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;

private:
    void initComponent();
    void runExternalApp();
    void initSearchText();

    //点击item 连接/断开
    void activeConnect(QString netName, QString deviceName, int type);
    void deActiveConnect(QString netName, QString deviceName, int type);



    int  sortWlanNet(QString deviceName, QString name, QString signal);
    void updateIcon(WlanItem *item, QString signalStrength, QString security);
    void resortWifiList(ItemFrame *frame, QVector<QStringList> list);


    //单wifi图标
    int  setSignal(QString lv);
    QString wifiIcon(bool isLock, int strength);


    //开关相关
    void setSwitchStatus();
    void hideLayout(QVBoxLayout * layout);
    void showLayout(QVBoxLayout * layout);


    //获取设备列表
    void getDeviceList(QStringList &list);
    //初始化设备wifi列表
    void initNet();
    void initNetListFromDevice(QString deviceName);
    //处理列表 已连接
    void addActiveItem(ItemFrame *frame, QString devName, QStringList infoList);
    //处理列表 未连接
    void addCustomItem(ItemFrame *frame, QString devName, QStringList infoList);
    //增加设备
    void addDeviceFrame(QString devName);
    //减少设备
    void removeDeviceFrame(QString devName);
    //增加ap
    void addOneWlanFrame(ItemFrame *frame, QString deviceName, QString name, QString signal, QString uuid, bool isLock, bool status, int type);
    //减少ap
    void removeOneWlanFrame(ItemFrame *frame, QString deviceName, QString ssid);


    //单个wifi连接状态变化
    void itemActiveConnectionStatusChanged(WlanItem *item, int status);
protected:
    bool eventFilter(QObject *w,QEvent *e);

private:
    Ui::WlanConnect *ui;

    QString            pluginName;
    int                pluginType;
    QWidget            *pluginWidget;

    QDBusInterface     *m_interface = nullptr;

    QGSettings         *m_switchGsettings = nullptr;

    //设备列表
    QStringList deviceList;
    //设备名 + 单设备frame
    QMap<QString, ItemFrame *> deviceFrameMap;

    //QVector<QStringList>  wlanSignalList;
//    DeviceWlanlistInfo   deviceWlanlistInfo;

    QTimer * m_scanTimer = nullptr;
    QTimer * m_updateTimer = nullptr;
private:
    SwitchButton       *wifiSwtch;
    bool               mFirstLoad;

private slots:

    void onNetworkAdd(QString deviceName, QStringList wlanInfo);
    void onNetworkRemove(QString deviceName, QString wlannName);
    void onActiveConnectionChanged(QString deviceName, QString ssid, QString uuid, int status);

    void updateList();
    void onDeviceStatusChanged();

    void reScan();


};
#endif // WLANCONNECT_H
