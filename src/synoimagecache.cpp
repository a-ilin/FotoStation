/*
 * GNU General Public License (GPL)
 * Copyright (c) 2019 by Aleksei Ilin
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

#include "synosettings.h"
#include "synoimagecache.h"

SynoImageCache::SynoImageCache()
    : m_hitCount(0)
    , m_missCount(0)
{
    SynoSettings settings(QStringLiteral("performance"));
    int maximumSizeMb = qBound(0, settings.value(QStringLiteral("ramImageCacheMb"), 150).toInt() * 1024 * 1024, std::numeric_limits<int>::max());
    int maximumCountItems = qBound(0, settings.value(QStringLiteral("ramImageCacheItems"), 5000).toInt(), std::numeric_limits<int>::max());

    m_cache = CacheType(maximumSizeMb, maximumCountItems);
}

QImage SynoImageCache::object(const QString& url, const QSize& minimumSize)
{
    QImage value = m_cache.object(url);
    if (minimumSize.height() <= value.size().height()
        && minimumSize.width() <= value.size().width()) {
        ++m_hitCount;
        return value;
    }
    ++m_missCount;
    return QImage();
}

void SynoImageCache::insert(const QString& url, const QImage& image)
{
    QImage value = m_cache.object(url);
    if (value.size().height() < image.size().height()
        && value.size().width() < image.size().width()) {
        m_cache.insert(url, image, image.sizeInBytes());
    }
}

void SynoImageCache::remove(const QString& url)
{
    m_cache.remove(url);
}

void SynoImageCache::clear()
{
    m_cache.clear();
}

SynoImageCache::CacheType::size_type SynoImageCache::count() const
{
    return m_cache.count();
}

SynoImageCache::CacheType::size_type SynoImageCache::totalCost() const
{
    return m_cache.totalCost();
}

quint64 SynoImageCache::hitCount() const
{
    return m_hitCount;
}

quint64 SynoImageCache::missCount() const
{
    return m_missCount;
}

