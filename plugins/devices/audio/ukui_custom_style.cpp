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
#include "ukui_custom_style.h"
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QPaintEvent>
#include <QStylePainter>
#include <QCoreApplication>
#include <QDebug>

UkuiMediaSliderTipLabel::UkuiMediaSliderTipLabel(){
    setAttribute(Qt::WA_TranslucentBackground);
}

UkuiMediaSliderTipLabel::~UkuiMediaSliderTipLabel(){
}

void UkuiMediaSliderTipLabel::paintEvent(QPaintEvent *e)
{
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    QStylePainter p(this);
//    p.setBrush(QBrush(QColor(0x1A,0x1A,0x1A,0x4C)));
    p.setBrush(QBrush(QColor(0xFF,0xFF,0xFF,0x33)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(this->rect(), 1, 1);
    QPainterPath path;
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHint(QPainter::Antialiasing);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    this->setProperty("blurRegion", QRegion(QRect(0, 0, 1, 1)));
    QLabel::paintEvent(e);
}

UkmediaVolumeSlider::UkmediaVolumeSlider(QWidget *parent,bool needTip)
{
    Q_UNUSED(parent);
    if (needTip) {
        state = needTip;
        m_pTiplabel = new UkuiMediaSliderTipLabel();
        m_pTiplabel->setWindowFlags(Qt::ToolTip);
    //    qApp->installEventFilter(new AppEventFilter(this));
        m_pTiplabel->setFixedSize(52,30);
        m_pTiplabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }
}

void UkmediaVolumeSlider::mousePressEvent(QMouseEvent *ev)
{
    if (state) {
        m_pTiplabel->show();
    }
    //注意应先调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
    QSlider::mousePressEvent(ev);
    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
    double pos = ev->pos().x() / (double)width();
    setValue(pos *(maximum() - minimum()) + minimum());
    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(parentWidget(), &evEvent);
}

void UkmediaVolumeSlider::initStyleOption(QStyleOptionSlider *option)
{
    QSlider::initStyleOption(option);
}

void UkmediaVolumeSlider::leaveEvent(QEvent *e)
{
    if (state) {
        m_pTiplabel->hide();
    }
}

void UkmediaVolumeSlider::enterEvent(QEvent *e)
{
    if (state) {
        m_pTiplabel->show();
    }
}

void UkmediaVolumeSlider::paintEvent(QPaintEvent *e)
{
    QRect rect;
    QStyleOptionSlider m_option;
    QSlider::paintEvent(e);
    if (state) {

        this->initStyleOption(&m_option);
        rect = this->style()->subControlRect(QStyle::CC_Slider, &m_option,QStyle::SC_SliderHandle,this);
        QPoint gPos = this->mapToGlobal(rect.topLeft());
        QString percent;
        percent = QString::number(this->value());
        percent.append("%");
        m_pTiplabel->setText(percent);
        m_pTiplabel->move(gPos.x()-(m_pTiplabel->width()/2)+9,gPos.y()-m_pTiplabel->height()-1);
    }


}

UkmediaVolumeSlider::~UkmediaVolumeSlider()
{
}

void UkuiButtonDrawSvg::init(QImage img, QColor color)
{
    mImage = img;
    mColor = color;
}

void UkuiButtonDrawSvg::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setBrush(QBrush(QColor(0x13,0x13,0x14,0x00)));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,6,6);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QRect UkuiButtonDrawSvg::IconGeometry()
{
    QRect res = QRect(QPoint(0,0),QSize(24,24));
    res.moveCenter(QRect(0,0,width(),height()).center());
    return res;
}

void UkuiButtonDrawSvg::draw(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    QRect iconRect = IconGeometry();
    if (mImage.size() != iconRect.size())
    {
        mImage = mImage.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect r = mImage.rect();
        r.moveCenter(iconRect.center());
        iconRect = r;
    }

    this->setProperty("fillIconSymbolicColor", true);
    filledSymbolicColoredPixmap(mImage,mColor);
    painter.drawImage(iconRect, mImage);
}

bool UkuiButtonDrawSvg::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Paint:
        draw(static_cast<QPaintEvent*>(event));
        break;

    case QEvent::Move:
    case QEvent::Resize:
    {
        QRect rect = IconGeometry();
    }
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        event->accept();
        break;

    default:
        break;
    }

    return QPushButton::event(event);
}


UkuiButtonDrawSvg::UkuiButtonDrawSvg(QWidget *parent)
{
    Q_UNUSED(parent);
}
UkuiButtonDrawSvg::~UkuiButtonDrawSvg()
{

}

QPixmap UkuiButtonDrawSvg::filledSymbolicColoredPixmap(QImage &img, QColor &baseColor)
{

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                int hue = color.hue();
                if (!qAbs(hue - symbolic_color.hue()) < 10) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }

    return QPixmap::fromImage(img);
}

CustomSound::CustomSound()
{
}

CustomSound::~CustomSound()
{
    delete(file);
    delete(doc);
    file->close();
}

bool CustomSound::createAudioFile()
{
    //打开或创建文件
    QFile *file;
    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    if(!QFile::exists(audioPath)){
        file = new QFile(audioPath);
        if(!file->open(QFile::WriteOnly))
            return false;
        //写入xml头部
        QDomDocument doc;
        QDomProcessingInstruction instruction; //添加处理命令

        instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        //添加根节点
        QDomElement root = doc.createElement("root");
        doc.appendChild(root);
        //添加子节点
        QDomElement node = doc.createElement("firstRun");
        QDomElement init = doc.createElement("init"); //创建子元素
        QDomText text = doc.createTextNode("true");
        init.appendChild(text);
        node.appendChild(init);
        root.appendChild(node);

        //输出到文件
        QTextStream out_stream(file);
        doc.save(out_stream,4); //缩进4格
        file->close();
        return true;
    }
    return true;
}

//判断某个节点是否存在
bool CustomSound::isExist(QString nodeName)
{
    if (nodeName == "")
        return false;
    //打开文件
    QString errorStr;
        int errorLine;
        int errorCol;

    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    QFile file(audioPath);

    if(!file.exists())
        createAudioFile();

    if(file.open(QFile::ReadOnly)){
        QDomDocument doc;
        if(doc.setContent(&file,true,&errorStr,&errorLine,&errorCol)){
            file.close();
            QDomElement root = doc.documentElement();
            QDomElement ele = root.firstChildElement();
            nodeName.remove(" ");
            nodeName.remove("/");
            nodeName.remove("(");
            nodeName.remove(")");
            nodeName.remove("[");
            nodeName.remove("]");
            if(nodeName.at(0)>='0' && nodeName.at(0)<='9'){
                nodeName = "Audio_"+nodeName;
            }
            while(!ele.isNull()) {
                if(ele.nodeName() == nodeName)
                    return true;
                ele = ele.nextSiblingElement();
            }
        }
        else
        {
            qDebug() << errorStr << "line: " << errorLine << "col: " << errorCol;
        }

        file.close();
    }

    return false;
}

//添加第一个子节点及其子元素
int CustomSound::addXmlNode(QString nodeName, bool initState)
{
    //打开文件
    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    QFile file(audioPath);
    QDomDocument doc;//增加一个一级子节点以及元素
    if(file.open(QFile::ReadOnly)){
        if(doc.setContent(&file)){
            file.close();
        }
        else {
            file.close();
            return -1;
        }
    }
    else
        return -1;

    //添加新节点
    nodeName.remove(" ");
    nodeName.remove("/");
    nodeName.remove("(");
    nodeName.remove(")");
    nodeName.remove("[");
    nodeName.remove("]");
    if(nodeName.at(0)>='0' && nodeName.at(0)<='9'){
        nodeName = "Audio_"+nodeName;
    }
    QDomElement root=doc.documentElement();
    QDomElement node=doc.createElement(nodeName);

    QDomElement init=doc.createElement("init");
    QDomText text;
    if(initState)
        text = doc.createTextNode("true");
    else
        text = doc.createTextNode("false");
    init.appendChild(text);
    node.appendChild(init);
    root.appendChild(node);
    qDebug() << "addXmlNode" << nodeName ;
    //修改first-run状态
    QDomElement ele = root.firstChildElement();
    while(!ele.isNull()) {
        if(ele.nodeName() == "firstRun"){
            QString value = ele.firstChildElement().firstChild().nodeValue();
            if(value == "true")
                ele.firstChildElement().firstChild().setNodeValue("false");
        }
        ele = ele.nextSiblingElement();
    }

    if(file.open(QFile::WriteOnly|QFile::Truncate)) {
        //输出到文件
        QTextStream out_stream(&file);
        doc.save(out_stream,4); //缩进4格
        file.close();
    }

    return 0;
}

bool CustomSound::isFirstRun()
{

    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    QFile file(audioPath);
    if(!file.exists()){
        createAudioFile();
    }

    if(file.open(QFile::ReadOnly)){
        QDomDocument doc;
        if(doc.setContent(&file)){
            file.close();
            QDomElement root = doc.documentElement();
            QDomElement ele = root.firstChildElement();

            qDebug()<<"===================ele.nodeName() :"<<ele.nodeName() ;
            if(ele.nodeName() == "first-run"){
                QString value = ele.firstChildElement().firstChild().nodeValue();
                if(value == "true") {
                    file.close();
                    return true;
                }
                else {
                    file.close();
                    return false;
                }
            }
        }
        file.close();
    }
    return false;
}

