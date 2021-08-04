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
#include "deluserdialog.h"
#include "ui_deluserdialog.h"
#include "CloseButton/closebutton.h"

#include <QDebug>
#include <QtGlobal>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

DelUserDialog::DelUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DelUserDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    setupComonpent();
    setupConnect();
}

DelUserDialog::~DelUserDialog()
{
    delete ui;
    ui = nullptr;
}

void DelUserDialog::setupComonpent(){

    ui->titleLabel->setStyleSheet("QLabel{color: palette(windowText);}");
//    ui->closeBtn->setFlat(true);
//    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));
//    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
//                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
    ui->label->adjustSize();
    ui->label->setWordWrap(true);

    ui->buttonGroup->setId(ui->keepRadioBtn, 0);
    ui->buttonGroup->setId(ui->delRadioBtn, 1);

    ui->delRadioBtn->setChecked(true);

    ui->frame_2->installEventFilter(this);
    ui->frame_3->installEventFilter(this);
}

void DelUserDialog::setupConnect(){

    connect(ui->cancelPushBtn, SIGNAL(clicked()), this, SLOT(reject()));

    connect(ui->deleteBtn, &QPushButton::clicked, this, [=]{
        this->accept();
        emit removefile_send(ui->buttonGroup->checkedId());
    });

//    QSignalMapper * differSignalMapper = new QSignalMapper();
//    for (QAbstractButton * button : ui->buttonGroup->buttons()){
//        connect(button, SIGNAL(clicked()), differSignalMapper, SLOT(map()));
//        differSignalMapper->setMapping(button, button->text());
//    }

//#if QT_VERSION <= QT_VERSION_CHECK(5,12,0)
//    connect(differSignalMapper, static_cast<void(QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), [=](const QString key){
//#else
//    connect(differSignalMapper, QOverload<const QString &>::of(&QSignalMapper::mapped), [=](const QString key){
//#endif
//        this->accept();
//        bool removefile;
//        if (ui->removePushBtn->text() == key)
//            removefile = true;
//        else
//            removefile = false;
//        emit removefile_send(removefile, ui->usernameLabel->text());
//    });
}


void DelUserDialog::setUsername(QString username){
    QString title1 = tr("Delete the user '");
    QString title3 = tr("'and:");
    ui->titleLabel->setText(title1 + username + title3);
}


void DelUserDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());
    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();

}

bool DelUserDialog::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == ui->frame_2){
                ui->keepRadioBtn->setChecked(true);
            } else if (watched == ui->frame_3){
                ui->delRadioBtn->setChecked(true);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
