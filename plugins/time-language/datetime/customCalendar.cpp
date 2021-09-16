//qcustomcalendarwidget.cpp

#include "customCalendar.h"

#include <QLocale>
#include <QPainter>
#include <QTextCharFormat>
#include <QProxyStyle>
#include <QTableView>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QToolButton>
#include <QApplication>
#include <QSvgRenderer>

CustomCalendarWidget::CustomCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
//        setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    this->installEventFilter(this);
    this->setMinimumHeight(230);
    this->setMinimumWidth(350);
    this->setContentsMargins(12,12,12,12);
    QToolButton *left_button = this->findChild<QToolButton *>("qt_calendar_prevmonth");
    QToolButton *right_button = this->findChild<QToolButton *>("qt_calendar_nextmonth");
    left_button->setIcon(QIcon::fromTheme("ukui-start-symbolic"));
    right_button->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
}

CustomCalendarWidget::~CustomCalendarWidget()
{

}

void CustomCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    /* 绘制背景 */
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette().color(QPalette::Base));
    painter->drawRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(), 0, 0);
    painter->restore();
    if (date == selectedDate()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(palette().color(QPalette::Highlight));

        painter->drawRoundedRect(rect.x() + 5, rect.y() + 4, rect.width() - 10, rect.height() - 6, 6, 6);
        painter->setPen(QColor(255, 255, 255));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    } else {
        QCalendarWidget::paintCell(painter, rect, date);
    }
}


void CustomCalendarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
//    this->move(0,0);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QColor color = palette().color(QPalette::Dark);
    QRect rectBoxt = this->rect();
    painter.setBrush(Qt::NoBrush);
    painter.translate(1, 1);

    color.setAlpha(20);
    painter.setPen(color);
    painter.drawRoundedRect(rectBoxt.adjusted(0, +2, -2, -2), 6, 6);

    color.setAlpha(60);
    painter.setPen(color);
    painter.drawRoundedRect(rectBoxt.adjusted(+1, +3, -3, -3), 6, 6);

    color.setAlpha(80);
    painter.setPen(color);
    painter.drawRoundedRect(rectBoxt.adjusted(+2, +4, -4, -4), 6, 6);

    color.setAlpha(100);
    painter.setPen(color);
    painter.drawRoundedRect(rectBoxt.adjusted(+3, +5, -5, -5), 6, 6);
}

void CustomCalendarWidget::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return;
}

QPixmap CustomCalendarWidget::loadSvg(const QString &path, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap CustomCalendarWidget::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                QColor colorSet = palette().color(QPalette::ButtonText);
                color.setRed(colorSet.red());
                color.setGreen(colorSet.green());
                color.setBlue(colorSet.blue());
                img.setPixelColor(x, y, color);
            }
        }
    }
    return QPixmap::fromImage(img);
}
