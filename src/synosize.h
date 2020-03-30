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

#ifndef SYNOSIZE_H
#define SYNOSIZE_H

#include <QHash>
#include <QObject>
#include <QSize>

class QJSEngine;
class QQmlEngine;

class SynoSizeGadget : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SynoSizeGadget)

public:
    enum SynoSize {
        SIZE_P,     // PREVIEW
        SIZE_PL,    // LARGE_PREVIEW

        SIZE_S,
        SIZE_SM,
        SIZE_M,
        SIZE_B,
        SIZE_L,
        SIZE_XL,

        SIZEEND,    // enum border
        OVERSIZED   // bigger than XL
    };
    Q_ENUM(SynoSize)

public:
    /*!
     * \brief This method returns instance of this object
     */
    static SynoSizeGadget& instance();
    static QObject* fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine);
    static QMetaEnum metaEnum();

    /*!
     * \brief This method returns size in pixels corresponding to the given SynoSize
     */
    Q_INVOKABLE QSize sizeBySyno(SynoSize size) const;

    /*!
     * \brief This method returns the smallest thumbnail size which includes the given size
     *
     * \param isPreview Shows that preview size should be returned
     *
     * If no thumbnail size could cover the passed size, OVERSIZED is returned
     */
    Q_INVOKABLE SynoSize fitSyno(int height, int width, bool isPreview = false) const;

private:
    explicit SynoSizeGadget();

private:
    /* hash < SynoSize : size > */
    QHash<int, int> m_sizes;
};

#endif // SYNOSIZE_H
