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
#include "netconnect.h"
#include "ui_netconnect.h"

#include "../shell/utils/utils.h"

#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QtDBus>
#include <QDir>
#include <QDebug>
#include <QtAlgorithms>

#define WIRED_TYPE          0
#define ITEMHEIGH           50
#define LAN_TYPE           0
#define CONTROL_CENTER_WIFI              "org.ukui.control-center.wifi.switch"

const QString KLanSymbolic      = "network-wired-connected-symbolic";
const QString NoNetSymbolic     = "network-wired-disconnected-symbolic";

const QString WIRED_SWITCH = "wiredswitch";
const QByteArray GSETTINGS_SCHEMA = "org.ukui.kylin-nm.switch";

#define ACTIVATING   1
#define ACTIVATED    2
#define DEACTIVATING 3
#define DEACTIVATED  4

bool sortByVal(const QPair<QString, int> &l, const QPair<QString, int> &r) {
    return (l.second < r.second);
}
NetConnect::NetConnect() :  mFirstLoad(true) {
    pluginName = tr("WiredConnect");
    pluginType = NETWORK;
}

NetConnect::~NetConnect() {
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
        delete m_interface;
        delete m_switchGsettings;
    } 
}

QString NetConnect::get_plugin_name() {
    return pluginName;
}

int NetConnect::get_plugin_type() {
    return pluginType;
}

QWidget *NetConnect::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::NetConnect;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        qDBusRegisterMetaType<QVector<QStringList>>();
        m_interface = new QDBusInterface("com.kylin.network",
                                         "/com/kylin/network",
                                         "com.kylin.network",
                                         QDBusConnection::sessionBus());
        if(!m_interface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
        initSearchText();
        initComponent();
    }
    return pluginWidget;
}

void NetConnect::plugin_delay_control() {

}

const QString NetConnect::name() const {

    return QStringLiteral("netconnect");
}

void NetConnect::initSearchText() {
    //~ contents_path /netconnect/Advanced settings"
    ui->detailBtn->setText(tr("Advanced settings"));
    ui->titleLabel->setText(tr("Wired Network"));
    //~ contents_path /netconnect/open
    ui->openLabel->setText(tr("open"));
}

bool NetConnect::eventFilter(QObject *w, QEvent *e) {
    if (e->type() == QEvent::Enter) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(button);border-radius:4px;}");
    } else if (e->type() == QEvent::Leave) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(base);border-radius:4px;}");
    }
    return QObject::eventFilter(w,e);
}

void NetConnect::initComponent() {
    wiredSwitch = new SwitchButton(pluginWidget);
    ui->openWIifLayout->addWidget(wiredSwitch);

    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        m_switchGsettings = new QGSettings(GSETTINGS_SCHEMA);
        connect(wiredSwitch, &SwitchButton::checkedChanged, this, [=] (bool checked) {
            if (!m_interface->isValid()) {
                return;
            }
            qDebug() << "[NetConnect]call setWiredSwitchEnable" << checked << __LINE__;
            m_interface->call(QStringLiteral("setWiredSwitchEnable"),checked);
            qDebug() << "[NetConnect]call setWiredSwitchEnable Respond"  << __LINE__;
        });
        //网络开关状态以及列表初始化
        setSwitchStatus();
        connect(m_switchGsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == WIRED_SWITCH) {
                setSwitchStatus();
            }
        });
    } else {
        wiredSwitch->blockSignals(true);
        wiredSwitch->setChecked(true);
        wiredSwitch->blockSignals(false);
        initNet();
        qDebug()<<"[Netconnect] org.ukui.kylin-nm.switch is not installed!";
    }

    getDeviceStatusMap(deviceStatusMap);
    if (deviceStatusMap.isEmpty()) {
        qDebug() << "[Netconnect] no device exist when init, set switch disable";
        wiredSwitch->setDisabledFlag(true);
    }
    initNet();

    // 有线网络断开或连接时刷新可用网络列表
    connect(m_interface, SIGNAL(lanActiveConnectionStateChanged(QString, QString, int)), this, SLOT(onActiveConnectionChanged(QString, QString, int)), Qt::QueuedConnection);
    //有线网络新增时添加网络
    connect(m_interface, SIGNAL(lanAdd(QString, QStringList)), this, SLOT(onLanAdd(QString, QStringList)), Qt::QueuedConnection);
    //删除有线网络
    connect(m_interface, SIGNAL(lanRemove(QString)), this, SLOT(onLanRemove(QString)), Qt::QueuedConnection);
    //更新有线网络
    connect(m_interface, SIGNAL(lanUpdate(QString, QStringList)), this, SLOT(updateLanInfo(QString, QStringList)));
    //网卡插拔处理
    connect(m_interface, SIGNAL(deviceStatusChanged()), this, SLOT(onDeviceStatusChanged()));

    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });
}

//获取网卡列表
void NetConnect::getDeviceStatusMap(QMap<QString, bool> &map)
{
    if (!m_interface->isValid()) {
        return;
    }
    qDebug() << "[NetConnect]call getDeviceListAndEnabled"  << __LINE__;
    QDBusMessage result = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),0);
    qDebug() << "[NetConnect]call getDeviceListAndEnabled Respond"  << __LINE__;
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "[NetConnect]getWiredDeviceList error:" << result.errorMessage();
        return;
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    dbusArg >> map;
}

//lanUpdate
void NetConnect::updateLanInfo(QString deviceName, QStringList lanInfo)
{
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (deviceName.isEmpty()) {
            //变为无指定网卡，所有列表都要添加
            if (!iter.value()->itemMap.contains(lanInfo.at(1)) && deviceStatusMap[lanInfo.at(1)]) {
                qDebug() << "[NetConnect]" << lanInfo.at(0) << " change to device none, add every list";
                addOneLanFrame(iter.value(), deviceName, lanInfo);
            }
        } else {
            if (iter.key() != deviceName) {
                qDebug() << "[NetConnect]" << lanInfo.at(0) << " not belongs to " << iter.key();
                removeOneLanFrame(iter.value(), deviceName, lanInfo.at(1));
            } else {
                if (!iter.value()->itemMap.contains(lanInfo.at(1)) && deviceStatusMap[lanInfo.at(1)]) {
                    qDebug() << "[NetConnect]" << lanInfo.at(0) << " now belongs to " << deviceName;
                    addOneLanFrame(iter.value(), deviceName, lanInfo);
                }
            }
        }
    }
}


//总开关
void NetConnect::setSwitchStatus()
{
    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        bool status = m_switchGsettings->get(WIRED_SWITCH).toBool();
        wiredSwitch->blockSignals(true);
        wiredSwitch->setChecked(status);
        wiredSwitch->blockSignals(false);
        if (!wiredSwitch->isChecked()) {
            hideLayout(ui->availableLayout);
        } else {
            showLayout(ui->availableLayout);
        }
    } else {
        qDebug()<<"[netconnect] org.ukui.kylin-nm.switch is not installed!";
    }

}

//总体隐藏
void NetConnect::hideLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i)
    {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->hide();
    }
}

//总体显示
void NetConnect::showLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i)
    {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->show();
    }
}

//初始化
void NetConnect::initNet()
{
    //先构建每个设备的列表头
    QStringList deviceList = deviceStatusMap.keys();
    for (int i = 0; i < deviceList.size(); ++i) {
        addDeviceFrame(deviceList.at(i));
    }
    //再填充每个设备的列表
    for (int i = 0; i < deviceList.size(); ++i) {
        if (deviceStatusMap[deviceList.at(i)]) {
            initNetListFromDevice(deviceList.at(i));
        }
    }
}

void NetConnect::runExternalApp() {
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

//激活
void NetConnect::activeConnect(QString ssid, QString deviceName, int type) {
    qDebug() << "[NetConnect]call activateConnect" << __LINE__;
    m_interface->call(QStringLiteral("activateConnect"),type, deviceName, ssid);
    qDebug() << "[NetConnect]call activateConnect respond" << __LINE__;
}

//断开
void NetConnect::deActiveConnect(QString ssid, QString deviceName, int type) {
    qDebug() << "[NetConnect]call deActivateConnect" << __LINE__;
    m_interface->call(QStringLiteral("deActivateConnect"),type, deviceName, ssid);
    qDebug() << "[NetConnect]call deActivateConnect respond" << __LINE__;
}

//初始化设备列表 网卡标号问题？
void NetConnect::initNetListFromDevice(QString deviceName)
{
    qDebug() << "[NetConnect]initNetListFromDevice " << deviceName;
    if (!wiredSwitch->isChecked()) {
         qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " switch off";
        return;
    }
    if (!deviceFrameMap.contains(deviceName)) {
        qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " not exist";
        return;
    }
    if (!m_interface->isValid()) {
        return;
    }
    qDebug() << "[NetConnect]call getWiredList"  << __LINE__;
    QDBusMessage result = m_interface->call(QStringLiteral("getWiredList"));
    qDebug() << "[NetConnect]call getWiredList respond"  << __LINE__;
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWiredList error:" << result.errorMessage();
        return;
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QVector<QStringList>> variantList;
    dbusArg >> variantList;
    if (variantList.size() == 0) {
        qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " list empty";
        return;
    }
    QMap<QString, QVector<QStringList>>::iterator iter;

    for (iter = variantList.begin(); iter != variantList.end(); iter++) {
        if (deviceName == iter.key()) {
            QVector<QStringList> wlanListInfo = iter.value();
            //处理列表 已连接
            qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " acitved lan " << wlanListInfo.at(0);
            addLanItem(deviceFrameMap[deviceName], deviceName,  wlanListInfo.at(0), true);
            //处理列表 未连接
            for (int i = 1; i < wlanListInfo.length(); i++) {
                qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " deacitved lan " << wlanListInfo.at(i);
                addLanItem(deviceFrameMap[deviceName], deviceName, wlanListInfo.at(i), false);
            }
        }
    }
    return;
}

//初始化时添加一个项 不考虑顺序
void NetConnect::addLanItem(ItemFrame *frame, QString devName, QStringList infoList, bool isActived)
{
    if (frame == nullptr) {
        return;
    }
    if (infoList.size() == 1) {
        return;
    }

    LanItem * lanItem = new LanItem(pluginWidget);
    QString iconPath;
    if (isActived) {
        iconPath = KLanSymbolic;
        lanItem->statusLabel->setText(tr("connected"));
    } else {
        iconPath = NoNetSymbolic;
        lanItem->statusLabel->setText("");
    }
    QIcon searchIcon = QIcon::fromTheme(iconPath);
    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
        lanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
    }
    lanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    lanItem->titileLabel->setText(infoList.at(0));

    lanItem->uuid = infoList.at(1);
    lanItem->dbusPath = infoList.at(2);

    connect(lanItem->infoLabel, &InfoButton::clicked, this, [=]{
        // open landetail page
        if (!m_interface->isValid()) {
            return;
        }
        qDebug() << "[NetConnect]call showPropertyWidget" << __LINE__;
        m_interface->call(QStringLiteral("showPropertyWidget"), devName, infoList.at(1));
        qDebug() << "[NetConnect]call showPropertyWidget respond" << __LINE__;
    });

    lanItem->isAcitve = isActived;

    connect(lanItem, &QPushButton::clicked, this, [=] {
        if (lanItem->isAcitve || lanItem->loading) {
            deActiveConnect(lanItem->uuid, devName, WIRED_TYPE);
        } else {
            activeConnect(lanItem->uuid, devName, WIRED_TYPE);
        }
    });

    //记录到deviceFrame的itemMap中
    deviceFrameMap[devName]->itemMap.insert(infoList.at(1), lanItem);
    qDebug()<<"insert " << infoList.at(1) << " to " << devName << " list";
    frame->lanItemLayout->addWidget(lanItem);
}

//增加设备
void NetConnect::addDeviceFrame(QString devName)
{
    qDebug() << "[NetConnect]addDeviceFrame " << devName;

    qDebug() << "[NetConnect]call getDeviceListAndEnabled"  << __LINE__;
    QDBusMessage result = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),0);
    qDebug() << "[NetConnect]call getDeviceListAndEnabled Respond"  << __LINE__;
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "[NetConnect]getWiredDeviceList error:" << result.errorMessage();
        return;
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString,bool> map;
    dbusArg >> map;

    bool enable = true;
    if (map.contains(devName)) {
        enable = map[devName];
    }

    ItemFrame *itemFrame = new ItemFrame(devName, pluginWidget);
    ui->availableLayout->addWidget(itemFrame);
    itemFrame->deviceFrame->deviceLabel->setText(tr("card")+/*QString("%1").arg(count)+*/"："+devName);
    itemFrame->deviceFrame->deviceSwitch->setChecked(enable);
    if (enable) {
        itemFrame->lanItemFrame->show();
    } else {
        itemFrame->lanItemFrame->hide();
    }
    deviceFrameMap.insert(devName, itemFrame);
    qDebug() << "[NetConnect]deviceFrameMap insert" << devName;

    connect(itemFrame->deviceFrame->deviceSwitch, &SwitchButton::checkedChanged, this, [=] (bool checked) {
        qDebug() << "[NetConnect]call setDeviceEnable" << devName << checked << __LINE__;
        m_interface->call(QStringLiteral("setDeviceEnable"), devName, checked);
        qDebug() << "[NetConnect]call setDeviceEnable Respond"  << __LINE__;
        if (checked) {
            itemFrame->lanItemFrame->show();
            initNetListFromDevice(devName);
        } else {
            if (itemFrame->lanItemFrame->layout() != NULL) {
                QLayoutItem* item;
                while ((item = itemFrame->lanItemFrame->layout()->takeAt(0)) != NULL) {
                    delete item->widget();
                    delete item;
                    item = nullptr;
                }
                itemFrame->itemMap.clear();
            }
        }
    });

    connect(itemFrame->addLanWidget, &AddNetBtn::clicked, this, [=](){
        if (m_interface->isValid()) {
            qDebug() << "[NetConnect]call showCreateWiredConnectWidget" << devName  << __LINE__;
            m_interface->call(QStringLiteral("showCreateWiredConnectWidget"), devName);
            qDebug() << "[NetConnect]call setDeviceEnable Respond"  << __LINE__;
        }
    });
}

//减少设备
void NetConnect::removeDeviceFrame(QString devName)
{
    qDebug() << "[NetConnect]removeDeviceFrame " << devName;
    if (deviceFrameMap.contains(devName)) {
        ItemFrame *item = deviceFrameMap[devName];
        delete item;
        item = nullptr;
        deviceFrameMap.remove(devName);
        qDebug() << "[NetConnect]deviceFrameMap remove" << devName;
    }
}

//device add or remove=================================
void NetConnect::onDeviceStatusChanged()
{
    qDebug()<<"[NetConnect]onDeviceStatusChanged";
    QEventLoop eventloop;
    QTimer::singleShot(300, &eventloop, SLOT(quit()));
    eventloop.exec();
    QStringList list;
    QMap<QString, bool> map;
    getDeviceStatusMap(map);
    list = map.keys();

    QStringList removeList;
    QMap<QString, bool> addMap;

    //remove的设备
    for (int i = 0; i< deviceStatusMap.keys().size(); ++i) {
        if (!list.contains(deviceStatusMap.keys().at(i))) {
            qDebug() << "[NetConnect]onDeviceStatusChanged " << deviceStatusMap.keys().at(i) << "was removed";
            removeList << deviceStatusMap.keys().at(i);
        }
    }

    //add的设备
    for (int i = 0; i< list.size(); ++i) {
        if (!deviceStatusMap.keys().contains(list.at(i))) {
            qDebug() << "[NetConnect]onDeviceStatusChanged " << list.at(i) << "was add, init status" << map[list.at(i)];
            addMap.insert(list.at(i),map[list.at(i)]);
        }
    }

    for (int i = 0; i < removeList.size(); ++i) {
        removeDeviceFrame(removeList.at(i));
    }

    QStringList addList = addMap.keys();
    for (int i = 0; i < addList.size(); ++i) {
        qDebug() << "add a device " << addList.at(i) << "status" << map[addList.at(i)];
        addDeviceFrame(addList.at(i));
        if (map[addList.at(i)]) {
            initNetListFromDevice(addList.at(i));
        }
    }
    deviceStatusMap = map;
    if (deviceStatusMap.isEmpty()) {
        wiredSwitch->setDisabledFlag(true);
    } else {
        wiredSwitch->setDisabledFlag(false);
    }
}

//wifi add===============================================================
void NetConnect::onLanAdd(QString deviceName, QStringList lanInfo)
{
    qDebug()<<"[NetConnect]onLanAdd "<< deviceName << " " << lanInfo;
    if(!wiredSwitch->isChecked()) {
        return;
    }

    if (!deviceName.isEmpty() && !deviceStatusMap.contains(deviceName)) {
        return;
    }

    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (deviceName.isEmpty()) {
            if (deviceStatusMap[iter.key()]) {
                qDebug() << "[NetConnect]onLanAdd every list" << iter.key();
                addOneLanFrame(iter.value(), iter.key(), lanInfo);
            }
        } else if (deviceName == iter.key()) {
            if (deviceStatusMap[deviceName]) {
                qDebug() << "[NetConnect]onLanAdd "<< deviceName;
                addOneLanFrame(iter.value(), deviceName, lanInfo);
                break;
            }
        }
    }
}

//wifi remove =============================================================
void NetConnect::onLanRemove(QString lanPath)
{
    //开关已关闭 忽略
//    if (!wifiSwtch->isChecked()) {
//        qDebug() << "[NetConnect]recieve network remove,but wireless switch is off";
//        return;
//    }

    qDebug()<<"[NetConnect]lan remove " << "dbus path:" << lanPath;
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        QMap<QString, LanItem *>::iterator itemIter;
        for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
            if (itemIter.value()->dbusPath == lanPath) {
               qDebug()<<"[NetConnect]lan remove " << lanPath << " find in " << itemIter.value()->titileLabel->text();
               QString key = itemIter.key();
               iter.value()->lanItemLayout->removeWidget(itemIter.value());
               delete itemIter.value();
               iter.value()->itemMap.remove(key);
               break;
            }
        }
    }
}

//增加一项
void NetConnect::addOneLanFrame(ItemFrame *frame, QString deviceName, QStringList infoList)
{
    if (nullptr == frame) {
        return;
    }

    if (frame->itemMap.contains(infoList.at(1))) {
            qDebug() << "[NetConnect]Already exist a lan " << infoList.at(1) << " in " << deviceName;
            return;
    }

    qDebug() << "[NetConnect]addOneLanFrame" << deviceName << infoList.at(0);
    QString connName = infoList.at(0);
    QString connUuid = infoList.at(1);
    QString connDbusPath = infoList.at(2);
    LanItem * lanItem = new LanItem(pluginWidget);

    QString iconPath;
    iconPath = NoNetSymbolic;
    lanItem->statusLabel->setText("");

    QIcon searchIcon = QIcon::fromTheme(iconPath);
    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
        lanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
    }
    lanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    lanItem->titileLabel->setText(connName);

    lanItem->uuid = connUuid;
    lanItem->dbusPath = connDbusPath;

    connect(lanItem->infoLabel, &InfoButton::clicked, this, [=]{
        // open landetail page
        if (!m_interface->isValid()) {
            return;
        }
        qDebug() << "[NetConnect]call showPropertyWidget" << deviceName << connUuid << __LINE__;
        m_interface->call(QStringLiteral("showPropertyWidget"), deviceName, connUuid);
        qDebug() << "[NetConnect]call showPropertyWidget respond" << __LINE__;
    });

    lanItem->isAcitve = false;

    connect(lanItem, &QPushButton::clicked, this, [=] {
        if (lanItem->isAcitve || lanItem->loading) {
            deActiveConnect(lanItem->uuid, deviceName, WIRED_TYPE);
        } else {
            activeConnect(lanItem->uuid, deviceName, WIRED_TYPE);
        }
    });

    //记录到deviceFrame的itemMap中
    deviceFrameMap[deviceName]->itemMap.insert(connUuid, lanItem);
    int index = getInsertPos(connName, deviceName);
    qDebug()<<"[NetConnect]addOneLanFrame " << connName << " to " << deviceName << " list at pos:" << index;
    frame->lanItemLayout->insertWidget(index, lanItem);
}

void NetConnect::removeOneLanFrame(ItemFrame *frame, QString deviceName, QString uuid)
{
    if (nullptr == frame) {
        return;
    }

    if (!frame->itemMap.contains(uuid)) {
            qDebug() << "[NetConnect]not exist a lan " << uuid << " in " << deviceName;
            return;
    }

   qDebug()<<"[NetConnect]removeOneLanFrame " << uuid << " find in " << deviceName;

   frame->lanItemLayout->removeWidget(frame->itemMap[uuid]);
   delete frame->itemMap[uuid];
   frame->itemMap.remove(uuid);
}

//activeconnect status change
void NetConnect::onActiveConnectionChanged(QString deviceName, QString uuid, int status)
{
    if (!wiredSwitch->isChecked()) {
        qDebug() << "[NetConnect]onActiveConnectionChanged but wiredSwitch is off";
        return;
    }
    if (uuid.isEmpty()) {
        qDebug() << "[NetConnect]onActiveConnectionChanged but uuid is empty";
        return;
    }
    qDebug() << "[NetConnect]onActiveConnectionChanged " << deviceName << uuid << status;
    LanItem * item= nullptr;
    if (deviceName.isEmpty()) {
        if (status != DEACTIVATED) {
            return;
        }
        //断开时 设备为空 说明此有线未指定设备 添加到所有列表中
        QStringList infoList;
        QMap<QString, ItemFrame *>::iterator iters;
        for (iters = deviceFrameMap.begin(); iters != deviceFrameMap.end(); iters++) {
            if (iters.value()->itemMap.contains(uuid)) {
                item = iters.value()->itemMap[uuid];
                infoList << item->titileLabel->text() << item->uuid << item->dbusPath;
                //为断开则重新插入
                int index = getInsertPos(item->titileLabel->text(), iters.key());
                qDebug() << "[NetConnect]reinsert" << item->titileLabel->text() << "pos" << index << "in" << iters.key() << "because status changes to deactive";
                deviceFrameMap[iters.key()]->lanItemLayout->removeWidget(item);
                deviceFrameMap[iters.key()]->lanItemLayout->insertWidget(index,item);
                itemActiveConnectionStatusChanged(item, status);
            }
        }
        if (!infoList.isEmpty()) {
            QMap<QString, ItemFrame *>::iterator iter;
            for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
                if (!iter.value()->itemMap.contains(uuid)) {
                    if (deviceStatusMap[iter.key()]) {
                        addOneLanFrame(iter.value(), iter.key(), infoList);
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < deviceFrameMap[deviceName]->itemMap.size(); ++i) {
            if (deviceFrameMap[deviceName]->itemMap.contains(uuid)) {
                item = deviceFrameMap[deviceName]->itemMap[uuid];
                if (status == ACTIVATED) {
                    //为已连接则放到第一个
                    deviceFrameMap[deviceName]->lanItemLayout->removeWidget(item);
                    deviceFrameMap[deviceName]->lanItemLayout->insertWidget(0,item);
                } else if (status == DEACTIVATED) {
                    //为断开则重新插入
                    int index = getInsertPos(item->titileLabel->text(), deviceName);
                    qDebug() << "[NetConnect]reinsert" << item->titileLabel->text() << "pos" << index << "in" << deviceName << "because status changes to deactive";
                    deviceFrameMap[deviceName]->lanItemLayout->removeWidget(item);
                    deviceFrameMap[deviceName]->lanItemLayout->insertWidget(index,item);
                }
                itemActiveConnectionStatusChanged(item, status);
            }
        }
    }
}

void NetConnect::itemActiveConnectionStatusChanged(LanItem *item, int status)
{
    QString iconPath = NoNetSymbolic;
    if (status == ACTIVATING) {
        item->setCountCurrentTime(0);
        item->setWaitPage(1);
        item->startLoading();
    } else if (status == ACTIVATED) {
        item->stopLoading();
        iconPath = KLanSymbolic;
        item->statusLabel->setStyleSheet("");
        item->statusLabel->setMinimumSize(36,36);
        item->statusLabel->setMaximumSize(16777215,16777215);
        item->statusLabel->setText(tr("connected"));
        item->isAcitve = true;
    } else if (status == DEACTIVATING) {
        item->setCountCurrentTime(0);
        item->setWaitPage(1);
        item->startLoading();
    } else {
        item->stopLoading();
        item->statusLabel->setStyleSheet("");
        item->statusLabel->setMinimumSize(36,36);
        item->statusLabel->setMaximumSize(16777215,16777215);
        item->statusLabel->setText("");
        item->isAcitve = false;
    }

    QIcon searchIcon = QIcon::fromTheme(iconPath);
    item->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
}

int NetConnect::getInsertPos(QString connName, QString deviceName)
{
    qDebug() << "[NetConnect]getInsertPos" << connName << deviceName;
    int index = 0;
    if(!m_interface->isValid()) {
        index = 0;
    } else {
        qDebug() << "[NetConnect]call getWiredList"  << __LINE__;
        QDBusMessage result = m_interface->call(QStringLiteral("getWiredList"));
        qDebug() << "[NetConnect]call getWiredList respond"  << __LINE__;
        if(result.type() == QDBusMessage::ErrorMessage)
        {
            qWarning() << "getWiredList error:" << result.errorMessage();
            return 0;
        }
        auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QVector<QStringList>> variantList;
        dbusArg >> variantList;
        for (int i = 0; i < variantList[deviceName].size(); ++i ) {
            if (variantList[deviceName].at(i).at(0) == connName) {
                qDebug() << "pos in kylin-nm is " << i;
                index = i;
                break;
            }
        }
        if (variantList[deviceName].at(0).size() == 1) {
            index--;
        }
    }
    return index;
}
