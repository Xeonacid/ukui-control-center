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

#ifndef COLORINFO_H
#define COLORINFO_H

#include <QHash>
#include <QVariant>
#include <QString>
#include <QDBusArgument>

struct ColorInfo {
    QString arg;
    QDBusVariant out;
};

QDBusArgument &operator<<(QDBusArgument &argument, const ColorInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.arg << mystruct.out;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ColorInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.arg >> mystruct.out;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(ColorInfo)

#endif // COLORINFO_H
