#include "changeusernickname.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

#include <QDBusInterface>
#include <QDBusReply>

extern "C" {
#include <glib.h>
}


ChangeUserNickname::ChangeUserNickname(QString nn, QStringList ns, QString op, QWidget *parent) :
    QDialog(parent),
    namesIsExists(ns),
    realname(nn)
{
    setFixedSize(QSize(480, 296));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
//    setAttribute(Qt::WA_DeleteOnClose);

    cniface = new QDBusInterface("org.freedesktop.Accounts",
                                  op,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus());


    initUI();
    setConnect();
    setupStatus();
}

ChangeUserNickname::~ChangeUserNickname()
{
    delete cniface;
}

void ChangeUserNickname::initUI(){

    //右上角关闭按钮
    closeBtn = new QPushButton();
    closeBtn->setFixedSize(QSize(14, 14));

    titleHorLayout = new QHBoxLayout;
    titleHorLayout->setSpacing(0);
    titleHorLayout->setContentsMargins(0, 0, 14, 0);
    titleHorLayout->addStretch();
    titleHorLayout->addWidget(closeBtn);

    //用户名
    userNameLabel = new QLabel();
    userNameLabel->setMinimumWidth(60);
    userNameLabel->setMaximumWidth(120);
    userNameLabel->setText(tr("UserName"));

    userNameLineEdit = new QLineEdit();
    userNameLineEdit->setFixedSize(QSize(300, 36));
    userNameLineEdit->setPlaceholderText(QString(g_get_user_name()));
    userNameLineEdit->setReadOnly(true);

    userNameHorLayout = new QHBoxLayout;
    userNameHorLayout->setSpacing(25);
    userNameHorLayout->setContentsMargins(0, 0, 0, 0);
    userNameHorLayout->addWidget(userNameLabel);
    userNameHorLayout->addWidget(userNameLineEdit);

    //用户昵称
    nickNameLabel = new QLabel();
    nickNameLabel->setMinimumWidth(60);
    nickNameLabel->setMaximumWidth(120);
    nickNameLabel->setText(tr("NickName"));

    tipLabel = new QLabel();
    tipLabel->setFixedSize(QSize(300, 24));
    QString tipinfo = tr("Name already in use, change another one.");
    if (setTextDynamicInNick(tipLabel, tipinfo)){
        tipLabel->setToolTip(tipinfo);
    }

    nickNameLineEdit = new QLineEdit();
    nickNameLineEdit->setFixedSize(QSize(300, 36));
    nickNameLineEdit->setText(realname);

    nickNameHorLayout = new QHBoxLayout;
    nickNameHorLayout->setSpacing(25);
    nickNameHorLayout->setContentsMargins(0, 0, 0, 0);
    nickNameHorLayout->addWidget(nickNameLabel);
    nickNameHorLayout->addWidget(nickNameLineEdit);

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(0, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    nickNameWithTipVerLayout = new QVBoxLayout;
    nickNameWithTipVerLayout->setSpacing(4);
    nickNameWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    nickNameWithTipVerLayout->addLayout(nickNameHorLayout);
    nickNameWithTipVerLayout->addLayout(tipHorLayout);

    //计算机名
    computerNameLabel = new QLabel();
    computerNameLabel->setMinimumWidth(60);
    computerNameLabel->setMaximumWidth(120);
    computerNameLabel->setText(tr("ComputerName"));

    computerNameLineEdit = new QLineEdit();
    computerNameLineEdit->setFixedSize(QSize(300, 36));
    computerNameLineEdit->setPlaceholderText(QString(g_get_host_name()));
    computerNameLineEdit->setReadOnly(true);

    computerNameHorLayout = new QHBoxLayout;
    computerNameHorLayout->setSpacing(25);
    computerNameHorLayout->setContentsMargins(0, 0, 0, 0);
    computerNameHorLayout->addWidget(computerNameLabel);
    computerNameHorLayout->addWidget(computerNameLineEdit);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(10);
    contentVerLayout->setContentsMargins(24, 0, 35, 24);
    contentVerLayout->addLayout(userNameHorLayout);
    contentVerLayout->addLayout(nickNameWithTipVerLayout);
    contentVerLayout->addLayout(computerNameHorLayout);

    //底部“取消”、“确定”按钮
    cancelBtn = new QPushButton();
    cancelBtn->setMinimumWidth(96);
    confirmBtn = new QPushButton();
    confirmBtn->setMinimumWidth(96);

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 25, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    //主布局
    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(20);
    mainVerLayout->setContentsMargins(0, 0, 0, 24);
    mainVerLayout->addLayout(titleHorLayout);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    setLayout(mainVerLayout);

}

void ChangeUserNickname::setConnect(){
    nickNameLineEdit->installEventFilter(this);

    connect(nickNameLineEdit, &QLineEdit::editingFinished, this, [=]{
        if (nickNameLineEdit->text().isEmpty()){
            nickNameLineEdit->setText(QString(g_get_real_name()));
        }
    });

    connect(nickNameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (namesIsExists.contains(txt)){
            tipLabel->show();
            confirmBtn->setEnabled(false);
        } else {
            tipLabel->hide();
            confirmBtn->setEnabled(true);
        }
    });

    connect(closeBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(confirmBtn, &QPushButton::clicked, this, [=]{
        cniface->call("SetRealName", nickNameLineEdit->text());
        close();
    });
}

void ChangeUserNickname::setupStatus(){
    tipLabel->hide();
}

bool ChangeUserNickname::setTextDynamicInNick(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    int pSize = label->width();
    if (fontSize > pSize) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, pSize);
        isOverLength = true;
    } else {

    }
    label->setText(str);
    return isOverLength;

}

void ChangeUserNickname::keyPressEvent(QKeyEvent * event){
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    case Qt::Key_Enter:
        if (confirmBtn->isEnabled())
            confirmBtn->clicked();
        break;
    case Qt::Key_Return:
        if (confirmBtn->isEnabled())
            confirmBtn->clicked();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}


bool ChangeUserNickname::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == nickNameLineEdit){
                if (QString::compare(nickNameLineEdit->text(), g_get_real_name()) == 0){
                    nickNameLineEdit->setText("");
                }
            }
        }
    }
}
