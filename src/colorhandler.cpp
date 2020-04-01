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

#include "colorhandler_p.h"
#include "colorhandlercms.h"

ColorHandler::ColorHandler(QObject *parent)
    : QObject(*new ColorHandlerPrivate(), parent)
{
}

QObject* ColorHandler::appWindow() const
{
    Q_D(const ColorHandler);

    return d->appWindow.data();
}

void ColorHandler::setAppWindow(QObject* value)
{
    Q_D(ColorHandler);

    if (d->appWindow != value) {
        if (d->appWindow) {
            disconnect(d->appWindow.data(), &QWindow::activeChanged, this, &ColorHandler::update);
            disconnect(d->appWindow.data(), &QWindow::visibleChanged, this, &ColorHandler::update);
            disconnect(d->appWindow.data(), &QWindow::screenChanged, this, &ColorHandler::update);
        }

        d->appWindow = qobject_cast<QWindow*>(value);

        if (d->appWindow) {
            connect(d->appWindow.data(), &QWindow::activeChanged, this, &ColorHandler::update);
            connect(d->appWindow.data(), &QWindow::visibleChanged, this, &ColorHandler::update);
            connect(d->appWindow.data(), &QWindow::screenChanged, this, &ColorHandler::update);
        }

        emit appWindowChanged();

        update();
    }
}

const QColorSpace& ColorHandler::colorSpace() const
{
    Q_D(const ColorHandler);

    ColorHandlerPrivate* d_self = const_cast<ColorHandlerPrivate*>(d);
    d_self->initializeColorSpace(false);
    return d->colorSpace;
}

bool ColorHandler::convertColorSpace(QImage& image, const QColorSpace& source, const QColorSpace& target)
{
#ifdef USE_LCMS2
    return ColorHandlerCms::instance().convertColorSpace(image, source, target);
#else
    if (!source.isValid() || !target.isValid() || source == target) {
        // formats are empty, or the same, do nothing
        return false;
    }

    image.convertToColorSpace(target);
    return true;
#endif
}

void ColorHandler::update()
{
    Q_D(ColorHandler);

    d->initializeColorSpace(true);
}

void ColorHandlerPrivate::initializeColorSpace(bool force)
{
    Q_Q(ColorHandler);

    if (iccProfile.isEmpty() || force) {
        QByteArray prevIccProfile = iccProfile;
        readPlatformIccProfile();

        if (iccProfile.isEmpty()) {
            // mark as broken to not run this method on each request
            iccProfile = QByteArrayLiteral("<bad>");
            colorSpace = QColorSpace();
            qWarning() << __FUNCTION__ << QStringLiteral("Cannot load ICC profile");
        } else if (iccProfile != prevIccProfile) {
            colorSpace = QColorSpace::fromIccProfile(iccProfile);
            if (!colorSpace.isValid()) {
                qWarning() << __FUNCTION__ << QStringLiteral("Cannot load ICC profile: ColorSpace is not valid");
            }
        }

        if (iccProfile != prevIccProfile) {
            emit q->colorSpaceChanged();
        }
    }
}
