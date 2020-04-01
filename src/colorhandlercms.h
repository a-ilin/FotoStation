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

#ifndef COLORHANDLERCMS_H
#define COLORHANDLERCMS_H

#include <memory>

#include <qglobal.h>

class QColorSpace;
class QImage;

class ColorHandlerCmsPrivate;
class ColorHandlerCms
{
    Q_DECLARE_PRIVATE(ColorHandlerCms)

public:
    static ColorHandlerCms& instance();

    bool convertColorSpace(QImage& image, const QColorSpace& source, const QColorSpace& target);

private:
    ColorHandlerCms();

private:
    std::unique_ptr<ColorHandlerCmsPrivate> d_ptr;
};

#endif // COLORHANDLERCMS_H
