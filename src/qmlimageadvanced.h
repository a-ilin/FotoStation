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

#ifndef QMLIMAGEADVANCED_H
#define QMLIMAGEADVANCED_H

#include <QtQuick/private/qquickimage_p.h>

#include <QQmlEngine>

class ColorHandler;

class QmlImageAdvancedPrivate;
class QmlImageAdvanced : public QQuickImage
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QmlImageAdvanced)
    Q_DISABLE_COPY(QmlImageAdvanced)

    QML_NAMED_ELEMENT(ImageAdvanced)

    Q_PROPERTY(QColorSpace windowColorSpace READ windowColorSpace WRITE setWindowColorSpace NOTIFY windowColorSpaceChanged)

public:
    QmlImageAdvanced(QQuickItem* parent = nullptr);

    const QColorSpace& windowColorSpace() const;
    void setWindowColorSpace(const QColorSpace& value);

signals:
    void windowColorSpaceChanged();

protected:
    void pixmapChange() override;
};

#endif // QMLIMAGEADVANCED_H
