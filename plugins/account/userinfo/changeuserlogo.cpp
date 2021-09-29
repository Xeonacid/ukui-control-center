#include "changeuserlogo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QDBusInterface>

#include <QDir>

#include "FlowLayout/flowlayout.h"


#define FACEPATH "/usr/share/ukui/faces/"

ChangeUserLogo::ChangeUserLogo(QString n, QString op, QWidget *parent) :
    QDialog(parent),
    name(n)
{
    setFixedSize(QSize(400, 462));

    culiface = new QDBusInterface("org.freedesktop.Accounts",
                                  op,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus());

    logosBtnGroup = new QButtonGroup;

    selected = QString();

    loadSystemLogo();
    initUI();
    setupConnect();
}

ChangeUserLogo::~ChangeUserLogo()
{
    delete culiface;
}

void ChangeUserLogo::loadSystemLogo(){

    logosFlowLayout = new FlowLayout(0, 5, 10);

    // 遍历头像目录
    QDir facesDir = QDir(FACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)) {
        QString fullface = QString("%1%2").arg(FACEPATH).arg(filename);
        if (fullface.endsWith(".svg"))
            continue;
        if (fullface.endsWith("3.png"))
            continue;

        QPushButton *button = new QPushButton;
        button->setCheckable(true);
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setFixedSize(QSize(64, 64));

        logosBtnGroup->addButton(button);

        QHBoxLayout *mainHorLayout = new QHBoxLayout(button);
        mainHorLayout->setSpacing(0);
        mainHorLayout->setMargin(0);
        QLabel *iconLabel = new QLabel(button);
//        iconLabel->setFixedSize(QSize(64, 64));
//        iconLabel->setPixmap(makeRoundLogo(fullface, iconLabel->width(), iconLabel->height(), iconLabel->width()/2));
        iconLabel->setScaledContents(true);
        iconLabel->setPixmap(QPixmap(fullface));

        mainHorLayout->addWidget(iconLabel);

        button->setLayout(mainHorLayout);

        connect(button, &QPushButton::clicked, [=]{
            // show dialog更新头像
            refreshUserLogo(fullface);

            selected = fullface;

            if (!culConfirmBtn->isEnabled())
                culConfirmBtn->setEnabled(true);
        });

        logosFlowLayout->addWidget(button);
    }

    logosFrame = new QFrame;
    logosFrame->setMinimumSize(QSize(352, 142));
    logosFrame->setMaximumSize(QSize(16777215, 142));
    logosFrame->setFrameShape(QFrame::Box);
    logosFrame->setFrameShadow(QFrame::Plain);
    logosFrame->setLayout(logosFlowLayout);
}


void ChangeUserLogo::initUI(){

    culLogoLabel = new QLabel;
    culLogoLabel->setFixedSize(QSize(48, 48));
    culNickNameLabel = new QLabel;
    culNickNameLabel->setFixedHeight(24);
    culTypeLabel = new QLabel;
    culTypeLabel->setFixedHeight(24);

    culUserInfoVerLayout = new QVBoxLayout;
    culUserInfoVerLayout->setSpacing(4);
    culUserInfoVerLayout->setContentsMargins(0, 0, 0, 0);
//    culUserInfoVerLayout->addStretch();
    culUserInfoVerLayout->addWidget(culNickNameLabel);
    culUserInfoVerLayout->addWidget(culTypeLabel);
//    culUserInfoVerLayout->addStretch();

    culUserHorLayout = new QHBoxLayout;
    culUserHorLayout->setSpacing(8);
    culUserHorLayout->setContentsMargins(0, 0, 0, 0);
    culUserHorLayout->addWidget(culLogoLabel);
    culUserHorLayout->addLayout(culUserInfoVerLayout);

    culNoteLabel = new QLabel;
    culNoteLabel->setFixedHeight(24);
    culNoteLabel->setText(tr("System Logos"));

    culLogoNoteHorLayout = new QHBoxLayout;
    culLogoNoteHorLayout->setSpacing(0);
    culLogoNoteHorLayout->setContentsMargins(0, 0, 0, 0);
    culLogoNoteHorLayout->addWidget(culNoteLabel);


    culMoreLogoBtn = new QPushButton;

    culMoreLogoHorLayout = new QHBoxLayout;
    culMoreLogoHorLayout->setSpacing(0);
    culMoreLogoHorLayout->setContentsMargins(0, 0, 0, 0);
    culMoreLogoHorLayout->addWidget(culMoreLogoBtn);
    culMoreLogoHorLayout->addStretch();

    culCancelBtn = new QPushButton;
    culConfirmBtn = new QPushButton;
    culConfirmBtn->setEnabled(false);

    culBottomBtnsHorLayout = new QHBoxLayout;
    culBottomBtnsHorLayout->setSpacing(16);
    culBottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    culBottomBtnsHorLayout->addStretch();
    culBottomBtnsHorLayout->addWidget(culCancelBtn);
    culBottomBtnsHorLayout->addWidget(culConfirmBtn);

    culMainVerLayout = new QVBoxLayout;
    culMainVerLayout->setSpacing(0);
    culMainVerLayout->setContentsMargins(24, 0, 24, 24);
    culMainVerLayout->addLayout(culUserHorLayout);
//    culMainVerLayout->addSpacing(35);
    culMainVerLayout->addLayout(culLogoNoteHorLayout);
    culMainVerLayout->addWidget(logosFrame);
    culMainVerLayout->addLayout(culMoreLogoHorLayout);
    culMainVerLayout->addSpacing(40);
    culMainVerLayout->addLayout(culBottomBtnsHorLayout);


    setLayout(culMainVerLayout);
}

QPixmap ChangeUserLogo::makeRoundLogo(QString logo, int wsize, int hsize, int radius){
    QPixmap rectPixmap;
    QPixmap iconcop = QPixmap(logo);

    if (iconcop.width() > iconcop.height()) {
        QPixmap iconPixmap = iconcop.copy((iconcop.width() - iconcop.height())/2, 0, iconcop.height(), iconcop.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(hsize);
    } else {
        QPixmap iconPixmap = iconcop.copy(0, (iconcop.height() - iconcop.width())/2, iconcop.width(), iconcop.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(wsize);
    }

    if (rectPixmap.isNull()) {
        return QPixmap();
    }
    QPixmap pixmapa(rectPixmap);
    QPixmap pixmap(radius*2,radius*2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius*2, radius*2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius*2, radius*2, pixmapa);
    return pixmap;
}

void ChangeUserLogo::refreshUserLogo(QString logo){
    culLogoLabel->setPixmap(makeRoundLogo(logo, culLogoLabel->width(), culLogoLabel->height(), culLogoLabel->width()/2));
}

void ChangeUserLogo::requireUserInfo(QString logo, QString type){
    //设置头像
    refreshUserLogo(logo);

    culNickNameLabel->setText(name);
    if (setCulTextDynamic(culNickNameLabel, name)){
        culNickNameLabel->setToolTip(name);
    }

    culTypeLabel->setText(type);
}

void ChangeUserLogo::setupConnect(){

    connect(culCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(culConfirmBtn, &QPushButton::clicked, this, [=]{
        culiface->call("SetIconFile", selected);
        close();
    });
}

bool ChangeUserLogo::setCulTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    if (fontSize > 80) {
        label->setFixedWidth(80);
        str = fontMetrics.elidedText(string, Qt::ElideRight, 80);
        isOverLength = true;
    } else {
        label->setFixedWidth(fontSize);
    }
    label->setText(str);
    return isOverLength;

}
