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

#ifndef SYNOALBUMFACTORY_H
#define SYNOALBUMFACTORY_H

#include <functional>

#include "synoalbum.h"
#include "synoalbumcache.h"

class QJSEngine;
class QQmlEngine;
class QmlObjectWrapper;

class SynoAlbumFactory : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SynoAlbumFactory)

public:
    /*!
     * \brief This method returns instance of album factory
     */
    static SynoAlbumFactory& instance();
    static QObject* fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine);

    /*!
     * \brief This method returns QmlObjectWrapper with album for the specified path
     *
     * \param path Path of the album
     *
     * \returns QmlObjectWrapper
     */
    Q_INVOKABLE QObject* createAlbumForPath(const QString& path = QString());

    /*!
     * \brief This method returns QmlObjectWrapper with album for the specified data
     *
     * \param synoData Data of the album
     *
     * \returns QmlObjectWrapper
     */
    Q_INVOKABLE QObject* createAlbumForData(const SynoAlbumData& data);

protected:
    SynoAlbumFactory();

    SynoAlbum* createRawAlbumForPath(const QString& path);
    SynoAlbum* createRawAlbumForData(const SynoAlbumData& data);

    QmlObjectWrapper* wrapFromCache(const QString& path, std::function<SynoAlbum*()> ctor);

protected:
    SynoAlbumCache m_cache;
};

#endif // SYNOALBUMFACTORY_H
