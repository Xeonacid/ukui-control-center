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
#include "biometrics.h"

Biometrics::Biometrics() : mFirstLoad(true)
{
    pluginName = tr("Biometrics");
    pluginType = ACCOUNT;
}

Biometrics::~Biometrics()
{

}

QString Biometrics::plugini18nName()
{
    return pluginName;
}

int Biometrics::pluginTypes()
{
    return pluginType;
}

QWidget *Biometrics::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        // will delete by takewidget
        pluginWidget = new BiometricsWidget;
    }

    return pluginWidget;
}

const QString Biometrics::name() const
{
    return QStringLiteral("Biometrics");
}

bool Biometrics::isShowOnHomePage() const
{
    return false;
}

QIcon Biometrics::icon() const
{
    return QIcon();
}

bool Biometrics::isEnable() const
{
    return true;
}