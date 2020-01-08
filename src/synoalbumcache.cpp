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

#include "synoalbumcache.h"
#include "synosettings.h"

SynoAlbumCache::SynoAlbumCache()
    : m_hitCount(0)
    , m_missCount(0)
{
    SynoSettings settings("performance");
    int maximumCountItems = qBound(0, settings.value("ramAlbumCacheItems", 50).toInt(), std::numeric_limits<int>::max());

    m_cache = CacheType(maximumCountItems);
}

 SynoAlbumCache::CacheType::mapped_type SynoAlbumCache::object(const QString& path)
{
    CacheType::mapped_type value = m_cache.object(path);
    if (value) {
        ++m_hitCount;
    } else {
        ++m_missCount;
    }
    return value;
}

void SynoAlbumCache::insert(const QString& path, SynoAlbumCache::CacheType::mapped_type album)
{
    m_cache.insert(path, album, 1);
}

void SynoAlbumCache::remove(const QString& path)
{
    m_cache.remove(path);
}

void SynoAlbumCache::clear()
{
    m_cache.clear();
}

SynoAlbumCache::CacheType::size_type SynoAlbumCache::count() const
{
    return m_cache.count();
}

quint64 SynoAlbumCache::hitCount() const
{
    return m_hitCount;
}

quint64 SynoAlbumCache::missCount() const
{
    return m_missCount;
}

