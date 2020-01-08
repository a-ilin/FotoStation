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

#ifndef SYNOALBUMCACHE_H
#define SYNOALBUMCACHE_H

#include <QObject>

#include "cache.h"

class SynoAlbumCache
{
    using CacheType = Cache< QString, std::shared_ptr<QObject> >;

public:
    SynoAlbumCache();

    CacheType::mapped_type object(const QString& path);
    void insert(const QString& path, CacheType::mapped_type album);
    void remove(const QString& path);
    void clear();

    /*! Returns amount of albums in cache */
    CacheType::size_type count() const;
    /*! Returns cache hit counter */
    quint64 hitCount() const;
    /*! Returns cache miss counter */
    quint64 missCount() const;

private:
    CacheType m_cache;
    quint64 m_missCount;
    quint64 m_hitCount;
};

#endif // SYNOALBUMCACHE_H
