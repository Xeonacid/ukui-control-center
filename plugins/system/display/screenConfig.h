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

#ifndef SCREENCONFIG_H
#define SCREENCONFIG_H

#include <QDBusArgument>
#include <QString>

struct ScreenConfig
{
    QString screenId;
    QString screenModeId;
    int screenPosX;
    int screenPosY;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const ScreenConfig &screenStruct)
    {
        argument.beginStructure();
        argument << screenStruct.screenId  << screenStruct.screenModeId << screenStruct.screenPosX << screenStruct.screenPosY;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ScreenConfig &screenStruct)
    {
        argument.beginStructure();
        argument >> screenStruct.screenId >> screenStruct.screenModeId >> screenStruct.screenPosX >> screenStruct.screenPosY;
        argument.endStructure();
        return argument;
    }
};
Q_DECLARE_METATYPE(ScreenConfig)

#endif // SCREENCONFIG_H
