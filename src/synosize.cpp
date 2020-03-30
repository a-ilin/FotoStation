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

#include <QQmlEngine>

#include "synosize.h"

SynoSizeGadget& SynoSizeGadget::instance()
{
    static SynoSizeGadget i;
    return i;
}

QObject* SynoSizeGadget::fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    SynoSizeGadget* i = &SynoSizeGadget::instance();
    QQmlEngine::setObjectOwnership(i, QQmlEngine::CppOwnership);
    return i;
}

QMetaEnum SynoSizeGadget::metaEnum()
{
    return QMetaEnum::fromType<SynoSizeGadget::SynoSize>();
}

QSize SynoSizeGadget::sizeBySyno(SynoSizeGadget::SynoSize size) const
{
    int sz;
    if (OVERSIZED == size) {
        sz = m_sizes.value(SIZE_XL);
    } else {
        sz = m_sizes.value(size);
    }

    return QSize(sz, sz);
}

SynoSizeGadget::SynoSize SynoSizeGadget::fitSyno(int height, int width, bool isPreview) const
{
    int bigger = std::max(height, width);

    if (isPreview) {
        if (bigger > m_sizes.value(SIZE_P)) {
            return SIZE_PL;
        }
        return SIZE_P;
    }

    for (int sizeEnum = SIZE_S; sizeEnum != SIZEEND; ++sizeEnum) {
        if (bigger <= m_sizes.value(sizeEnum)) {
            return static_cast<SynoSize>(sizeEnum);
        }
    }

    return OVERSIZED;
}

SynoSizeGadget::SynoSizeGadget()
    : QObject()
{
    m_sizes = {
        {SIZE_P,   160},
        {SIZE_PL, 2000},

        {SIZE_S,   120},
        {SIZE_SM,  240},
        {SIZE_M,   320},
        {SIZE_B,   640},
        {SIZE_L,   800},
        {SIZE_XL, 1280}
    };
}
