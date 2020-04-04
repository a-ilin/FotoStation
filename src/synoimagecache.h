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

#ifndef SYNOIMAGECACHE_H
#define SYNOIMAGECACHE_H

#include "cache.h"

#include <QImage>
#include <QString>
#include <QSize>

struct SynoImageCacheKey
{
    bool operator==(const SynoImageCacheKey& o) const {
        return id == o.id
            && synoSize == o.synoSize;
    }

    QString id;
    QByteArray synoSize;
};

inline uint qHash(const SynoImageCacheKey& key) {
    return qHash(key.id) ^ qHash(key.synoSize);
}

class SynoImageCache
{
    using CacheType = Cache< SynoImageCacheKey, QImage >;

public:
    SynoImageCache();

    QImage object(const QString& url,  const QByteArray& sizeId);
    void insert(const QString& url, const QByteArray& sizeId, const QImage& image);
    void remove(const QString& url, const QByteArray& sizeId);
    void clear();

    /*! Returns amount of images in cache */
    CacheType::size_type count() const;
    /*! Returns amount of used memory by images in cache in bytes */
    CacheType::size_type totalCost() const;
    /*! Returns cache hit counter */
    quint64 hitCount() const;
    /*! Returns cache miss counter */
    quint64 missCount() const;

private:
    CacheType m_cache;
    quint64 m_missCount;
    quint64 m_hitCount;
};

#endif // SYNOIMAGECACHE_H
