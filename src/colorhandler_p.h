/*
 * GNU General Public License (GPL)
 * Copyright (c) 2020 by Aleksei Ilin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COLORHANDLER_P_H
#define COLORHANDLER_P_H

#include "colorhandler.h"

#include <QtCore/private/qobject_p.h>

#include <QByteArray>
#include <QDebug>
#include <QImage>
#include <QPointer>
#include <QScreen>
#include <QWindow>

class ColorHandlerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(ColorHandler)

public:
    ColorHandlerPrivate()
        : QObjectPrivate()
    {
    }

    void initializeColorSpace(bool force);
    void readPlatformIccProfile();

    QPointer<QWindow> appWindow;
    QByteArray iccProfile;
    QColorSpace colorSpace;
};

#endif // COLORHANDLER_P_H
