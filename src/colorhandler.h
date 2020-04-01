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


#ifndef COLORHANDLER_H
#define COLORHANDLER_H

#include <QColorSpace>
#include <QObject>
#include <QQmlEngine>

class ColorHandlerPrivate;
class QImage;

class ColorHandler : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ColorHandler)
    Q_DISABLE_COPY(ColorHandler)

    QML_ELEMENT

    Q_PROPERTY(QObject* appWindow READ appWindow WRITE setAppWindow NOTIFY appWindowChanged)
    Q_PROPERTY(QColorSpace colorSpace READ colorSpace NOTIFY colorSpaceChanged)

public:
    explicit ColorHandler(QObject *parent = nullptr);

    QObject* appWindow() const;
    void setAppWindow(QObject* value);

    const QColorSpace& colorSpace() const;

    /*!
     * \brief Converts color space of image from source to target
     * \return Returns true if conversion was done, false otherwise
     */
    static bool convertColorSpace(QImage& image, const QColorSpace& source, const QColorSpace& target);

public slots:
    void update();

signals:
    void appWindowChanged();
    void colorSpaceChanged();
};

#endif // COLORHANDLER_H
