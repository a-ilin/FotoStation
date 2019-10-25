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


#ifndef SYNOIMAGEPROVIDER_H
#define SYNOIMAGEPROVIDER_H

#include "synoimagecache.h"

#include <QMutex>
#include <QQuickAsyncImageProvider>

class SynoConn;

class SynoImageProvider : public QObject, public QQuickAsyncImageProvider
{
    Q_OBJECT

public:
    class CacheLocker;

public:
    SynoImageProvider(SynoConn* conn);

    /*! Returns assigned connection object */
    SynoConn* conn() const;

    /*! Invalidates cached image by id */
    Q_INVOKABLE void invalidateInCache(const QString& id);

    // ImageProvider API
    QQuickImageResponse* requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    SynoConn* m_conn;
    // NOTE: SynoImageCache is not thread-safe
    QMutex m_imageCacheMutex;
    SynoImageCache m_imageCache;
};

class SynoImageProvider::CacheLocker
{
    Q_DISABLE_COPY(SynoImageProvider::CacheLocker)

public:
    CacheLocker(SynoImageProvider* provider);
    ~CacheLocker();

    SynoImageCache& cache();

private:
    SynoImageProvider* m_provider;
};


#endif // SYNOIMAGEPROVIDER_H
