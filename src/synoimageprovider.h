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

#include <QQuickAsyncImageProvider>

class SynoImageCache;
class SynoConn;
class SynoImageProviderPrivate;

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QtQuick/private/qquickpixmapcache_p.h>
class SynoImageProvider : public QObject, public QQuickImageProviderWithOptions
#else
class SynoImageProvider : public QObject, public QQuickAsyncImageProvider
#endif
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SynoImageProvider)

public:
    friend class SynoImageResponse;

public:
    SynoImageProvider(SynoConn* conn);
    ~SynoImageProvider();

    /*! Invalidates cached image by id */
    Q_INVOKABLE void invalidateInCache(const QString& id);

    // ImageProvider API
    QQuickImageResponse* requestImageResponse(const QString &id,
                                              const QSize &requestedSize,
                                              const QQuickImageProviderOptions &options) override;
};

#endif // SYNOIMAGEPROVIDER_H
