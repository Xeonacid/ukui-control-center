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
#ifndef NETCONNECT_H
#define NETCONNECT_H


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
#include <QMap>
#include <QPropertyAnimation>
#include <QPoint>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "shell/interface.h"
#include "AddBtn/addbtn.h"
#include "Label/fixlabel.h"
#include "SwitchButton/switchbutton.h"
#include "commonComponent/HoverBtn/hoverbtn.h"
#include "lanitem.h"
#include "deviceframe.h"
#include "itemframe.h"

enum {
    DISCONNECTED,
    NOINTERNET,
    CONNECTED
};

namespace Ui {
class NetConnect;
}

class NetConnect : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    NetConnect();
    ~NetConnect();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    void initSearchText();
    void initComponent();
    void runExternalApp();


    //开关相关
    void setSwitchStatus();
    void hideLayout(QVBoxLayout * layout);
    void showLayout(QVBoxLayout * layout);

    int getInsertPos(QString connName, QString deviceName);


    void deleteOneLan(QString ssid);
    void activeConnect(QString ssid, QString deviceName, int type);
    void deActiveConnect(QString ssid, QString deviceName, int type);

    //获取设备列表
    void getDeviceStatusMap(QMap<QString, bool> &map);
    void initNet();
    void initNetListFromDevice(QString deviceName);
    //处理列表增加
    void addLanItem(ItemFrame *frame, QString devName, QStringList infoList, bool isActived);
    //增加设备
    void addDeviceFrame(QString devName);
    //减少设备
    void removeDeviceFrame(QString devName);
    //增加一项
    void addOneLanFrame(ItemFrame *frame, QString devName, QStringList infoList);
    //减少一项
    void removeOneLanFrame(ItemFrame *frame, QString deviceName, QString uuid);

    //单个lan连接状态变化
    void itemActiveConnectionStatusChanged(LanItem *item, int status);

protected:
    bool eventFilter(QObject *w,QEvent *e);

private:
    Ui::NetConnect     *ui;

    QString            pluginName;
    int                pluginType;
    QWidget            *pluginWidget;

    QDBusInterface     *m_interface = nullptr;
    SwitchButton       *wiredSwitch;

    bool               mFirstLoad;
    QGSettings         *m_switchGsettings;

    QMap<QString, bool> deviceStatusMap;
    QMap<QString, ItemFrame *> deviceFrameMap;

private slots:
    void updateLanInfo(QString deviceName, QStringList lanInfo);

    void onLanAdd(QString deviceName, QStringList lanInfo);
    void onLanRemove(QString dbusPath);

    void onActiveConnectionChanged(QString deviceName, QString uuid, int status);

    void onDeviceStatusChanged();
};

Q_DECLARE_METATYPE(QList<QDBusObjectPath>);

#endif // NETCONNECT_H
