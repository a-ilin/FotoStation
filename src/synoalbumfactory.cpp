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

#include <QDebug>
#include <QQmlEngine>
#include <QTimer>

#include "qmlobjectwrapper.h"
#include "synoalbumfactory.h"
#include "synops.h"

SynoAlbumFactory& SynoAlbumFactory::instance()
{
    static SynoAlbumFactory i;
    return i;
}

QObject* SynoAlbumFactory::fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    SynoAlbumFactory* i = &SynoAlbumFactory::instance();
    QQmlEngine::setObjectOwnership(i, QQmlEngine::CppOwnership);
    return i;
}

QObject* SynoAlbumFactory::createAlbumForPath(const QString& path)
{
    return wrapFromCache(path, [&]() -> SynoAlbum* {
        return createRawAlbumForPath(path);
    });
}

QObject* SynoAlbumFactory::createAlbumForData(const SynoAlbumData& data)
{
    return wrapFromCache(data.path, [&]() -> SynoAlbum* {
        return createRawAlbumForData(data);
    });
}

SynoAlbumFactory::SynoAlbumFactory()
    : QObject()
{
    QTimer* cacheStatisticTimer = new QTimer(this);
    connect(cacheStatisticTimer, &QTimer::timeout, this, [this]() {
        qDebug() << tr("Album cache statistics. Count: %1. Hit: %2. Miss: %3.")
                    .arg(m_cache.count())
                    .arg(m_cache.hitCount()).arg(m_cache.missCount());
    });
    cacheStatisticTimer->start(60000);
}

SynoAlbum* SynoAlbumFactory::createRawAlbumForPath(const QString& path)
{
    return new SynoAlbum(SynoPS::instance()->conn(), path);
}

SynoAlbum* SynoAlbumFactory::createRawAlbumForData(const SynoAlbumData& data)
{
    return new SynoAlbum(SynoPS::instance()->conn(), data);
}

QmlObjectWrapper* SynoAlbumFactory::wrapFromCache(const QString& path, std::function<SynoAlbum* ()> ctor)
{
    std::shared_ptr<QObject> o = m_cache.object(path);
    if (!o) {
        SynoAlbum* album = ctor();
        QQmlEngine::setObjectOwnership(album, QQmlEngine::CppOwnership);
        o.reset(album);
        m_cache.insert(path, o);
    }

    return new QmlObjectWrapper(o);
}
