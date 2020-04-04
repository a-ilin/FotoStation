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


#ifndef SYNOIMAGEPROVIDER_P_H
#define SYNOIMAGEPROVIDER_P_H

#include "synoimagecache.h"
#include "synoimageprovider.h"
#include "synorequest.h"

#include <QtConcurrent>
#include <QColorSpace>
#include <QMutex>
#include <QQuickImageResponse>
#include <QThread>

#include <QtCore/private/qobject_p.h>

#include <atomic>

class SynoImageProviderPrivate : public QObjectPrivate
{
public:
    class CacheLocker;

public:
    SynoImageProviderPrivate()
        : QObjectPrivate() {}

public:
    SynoConn* conn = nullptr;

    // NOTE: SynoImageCache is not thread-safe
    QMutex imageCacheMutex;
    SynoImageCache imageCache;
    QThread thread;
};

class SynoImageResponse : public QQuickImageResponse
{
    Q_OBJECT

    enum CancelStatus {
        Status_NotCancelled = 0,
        Status_Cancelled,
        Statuc_CancelledConfirmed
    };

public:
    SynoImageResponse(SynoImageProvider* provider,
                      const QByteArray& id,
                      const QSize& size,
                      const QQuickImageProviderOptions& options);

    void load();

    QQuickTextureFactory* textureFactory() const override;
    QString errorString() const override;
    void cancel() override;

signals:
    void cacheCheckFinished(bool success);
    void requestCancelled();

protected:
    void setErrorString(const QString& err);
    bool loadFromCache();
    void sendRequest();
    void processNetworkRequest();
    void postProcessImage();

    void updateSynoThumbSize();

protected slots:
    void onCacheCheckFinished(bool success);

protected:
    SynoImageProvider* m_provider;
    QByteArray m_id;
    QSize m_size;
    QByteArray m_synoSize;
    QQuickImageProviderOptions m_options;
    QString m_errorString;
    QImage m_image;
    std::shared_ptr<SynoRequest> m_req;

    QFuture<void> m_future;
    std::atomic<CancelStatus> m_cancelStatus;
};

class SynoImageProviderPrivate::CacheLocker
{
    Q_DISABLE_COPY(SynoImageProviderPrivate::CacheLocker)

public:
    CacheLocker(SynoImageProviderPrivate* d);
    ~CacheLocker();

    SynoImageCache& cache();

private:
    SynoImageProviderPrivate* m_d;
};

#endif // SYNOIMAGEPROVIDER_P_H
