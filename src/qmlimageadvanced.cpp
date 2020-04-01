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

#include "qmlimageadvanced.h"

#include "colorhandler.h"

#include <QtQuick/private/qquickimage_p_p.h>

class QmlImageAdvancedPrivate : public QQuickImagePrivate
{
    Q_DECLARE_PUBLIC(QmlImageAdvanced)

public:
    void convertColor(bool recursive);

public:
    QColorSpace windowColorSpace;
};

QmlImageAdvanced::QmlImageAdvanced(QQuickItem* parent)
    : QQuickImage(*(new QmlImageAdvancedPrivate()), parent)
{

}

const QColorSpace& QmlImageAdvanced::windowColorSpace() const
{
    Q_D(const QmlImageAdvanced);

    return d->windowColorSpace;
}

void QmlImageAdvanced::setWindowColorSpace(const QColorSpace& value)
{
    Q_D(QmlImageAdvanced);

    if (d->windowColorSpace != value) {
        d->windowColorSpace = value;
        d->convertColor(false);
        emit windowColorSpaceChanged();
    }
}

void QmlImageAdvanced::pixmapChange()
{
    Q_D(QmlImageAdvanced);

    d->convertColor(true);
    QQuickImage::pixmapChange();
}

void QmlImageAdvancedPrivate::convertColor(bool recursive)
{
    Q_Q(QmlImageAdvanced);

    // TODO: optimize by reimplementation of QQuickTextureFactory
    // to eliminate double image conversion in main thread

    if (pix.isReady()) {
        QImage image(pix.image());
        if (!image.isNull() && ColorHandler::convertColorSpace(image, pix.colorSpace(), windowColorSpace)) {
            pix.setImage(image);
            q->setColorSpace(windowColorSpace);

            if (!recursive) {
                q->pixmapChange();
            }
        }
    }
}
