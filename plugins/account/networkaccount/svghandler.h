/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef QL_SVG_HANDLER_H
#define QL_SVG_HANDLER_H

#include <QObject>
#include <QPixmap>
#include <QSvgRenderer>
#include <QPainter>
#include <QApplication>
#include <QGSettings/QGSettings>

class SVGHandler : public QObject
{
    Q_OBJECT
public:
    explicit SVGHandler(QObject *parent = nullptr,bool highLight = false);
    const QPixmap loadSvg(const QString &fileName,int size = 24);
    const QPixmap loadSvgColor(const QString &path, const QString &color, int size = 48);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);
private:
    QGSettings *themeSettings;
    QString m_color;
Q_SIGNALS:

};

#endif // QL_SVG_HANDLER_H
