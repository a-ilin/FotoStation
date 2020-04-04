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

#include "synoimageprovider.h"
#include "synoimageprovider_p.h"
#include "colorhandler.h"
#include "synoconn.h"
#include "synops.h"
#include "synoreplyjson.h"
#include "synosize.h"

#include <QBuffer>
#include <QImageReader>
#include <QQuickTextureFactory>
#include <QTimer>

#include <functional>

SynoImageProvider::SynoImageProvider(SynoConn* conn)
    : QObject(*(new SynoImageProviderPrivate()), nullptr)
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    , QQuickImageProviderWithOptions(ImageResponse, ForceAsynchronousImageLoading)
#else
    , QQuickAsyncImageProvider()
#endif
{
    Q_D(SynoImageProvider);

    d->conn = conn;

    d->thread.setObjectName(QStringLiteral("SynoImageProviderThread"));
    d->thread.start();

    QTimer* cacheStatisticTimer = new QTimer(this);
    connect(cacheStatisticTimer, &QTimer::timeout, this, [this]() {
        SynoImageProviderPrivate::CacheLocker cacheLocker(d_func());
        SynoImageCache& cache = cacheLocker.cache();
        qDebug() << tr("Image cache statistics. Count: %1. Cost (KB): %2. Hit: %3. Miss: %4.")
                    .arg(cache.count()).arg(cache.totalCost() / 1024)
                    .arg(cache.hitCount()).arg(cache.missCount());
    });
    cacheStatisticTimer->start(60000);
}

SynoImageProvider::~SynoImageProvider()
{
    Q_D(SynoImageProvider);

    d->thread.quit();
    d->thread.wait();
}

void SynoImageProvider::invalidateInCache(const QString& id)
{
    Q_D(SynoImageProvider);

    SynoImageProviderPrivate::CacheLocker cacheLocker(d);
    cacheLocker.cache().remove(id);
}

QQuickImageResponse* SynoImageProvider::requestImageResponse(const QString& id,
                                                             const QSize& requestedSize,
                                                             const QQuickImageProviderOptions& options)
{
    Q_D(SynoImageProvider);

    SynoImageResponse* response = new SynoImageResponse(this, id.toLatin1(), requestedSize, options);
    response->moveToThread(&d->thread);
    QTimer::singleShot(0, response, &SynoImageResponse::load);
    return response;
}

SynoImageResponse::SynoImageResponse(SynoImageProvider* provider,
                                     const QByteArray& id,
                                     const QSize& size,
                                     const QQuickImageProviderOptions& options)
    : m_provider(provider)
    , m_id(id)
    , m_size(size)
    , m_options(options)
    , m_cancelStatus(Status_NotCancelled)
{
}

void SynoImageResponse::load()
{
    // it could be cancelled already

    CancelStatus cancel(Status_Cancelled);
    if (!m_cancelStatus.compare_exchange_strong(cancel, Statuc_CancelledConfirmed)) {
        connect(this, &SynoImageResponse::cacheCheckFinished, this, [this](bool success) {
            onCacheCheckFinished(success);
        });

        m_future = QtConcurrent::run([this]() {
            if (loadFromCache()) {
                postProcessImage();
                emit cacheCheckFinished(true);
            } else {
                emit cacheCheckFinished(false);
            }
        });
    } else {
        emit finished();
    }
}

void SynoImageResponse::sendRequest()
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=get");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("size=") + synoThumbSize();
    formData << QByteArrayLiteral("id=") + m_id;

    Q_ASSERT(!m_req);
    m_req = m_provider->d_func()->conn->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Thumb"), formData);

    connect(this, &SynoImageResponse::requestCancelled, this, [this]() {
        // it is safe to check here, as this code runs in object's thread
        if (!m_future.isRunning()) {
            emit finished();
        }
    });

    m_req->send(this, [this] {
        CancelStatus cancel(Status_Cancelled);
        if (!m_cancelStatus.compare_exchange_strong(cancel, Statuc_CancelledConfirmed)) {

            if (!m_req->errorString().isEmpty()) {
                setErrorString(tr("Network error: %1.").arg(m_req->errorString()));
            } else {
                if (m_req->contentType() == SynoRequest::TEXT) {
                    // some syno error happened
                    SynoReplyJSON replyJSON(m_req.get());
                    if (!replyJSON.errorString().isEmpty()) {
                        setErrorString(tr("Syno error: %1.").arg(replyJSON.errorString()));
                    } else {
                        setErrorString(tr("Unknown Syno error."));
                    }
                } else {
                    m_future = QtConcurrent::run([this]() {
                        processNetworkRequest();
                        postProcessImage();
                        emit finished();
                    });
                    return;
                }
            }
        }

        emit finished();
    });
}

QQuickTextureFactory* SynoImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QString SynoImageResponse::errorString() const
{
    return m_errorString;
}

void SynoImageResponse::cancel()
{
    CancelStatus cancel(Status_NotCancelled);
    if (m_cancelStatus.compare_exchange_strong(cancel, Status_Cancelled)) {
        if (m_req) {
            m_req->cancel();
            emit requestCancelled();
        }
    }
}

void SynoImageResponse::setErrorString(const QString& err)
{
    m_errorString = tr("Unable to obtain image. %1\nId: %2")
                    .arg(err).arg(QString::fromLatin1(m_id));
    qWarning() << __FUNCTION__ << m_errorString;
}

bool SynoImageResponse::loadFromCache()
{
    SynoImageProviderPrivate::CacheLocker cacheLocker(m_provider->d_func());
    QImage image = cacheLocker.cache().object(m_id, m_size);
    if (!image.isNull()) {
        m_image = image;
        return true;
    }

    return false;
}

void SynoImageResponse::processNetworkRequest()
{
    Q_ASSERT(m_req);

    // assume JPEG as the most widely used format
    QByteArray imageFormat(QByteArrayLiteral("JPG"));
    if (m_req->contentType() != SynoRequest::IMAGE_JPEG) {
        QList<QByteArray> imageFormatsForMimeType = QImageReader::imageFormatsForMimeType(m_req->contentMimeTypeRaw());
        if (imageFormatsForMimeType.size() > 0) {
            imageFormat = imageFormatsForMimeType[0];
        } else {
            // unknown mime type
            imageFormat = QByteArray();
        }
    }

    QByteArray data(m_req->replyBody());
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, imageFormat);

    if (reader.read(&m_image) && !m_image.isNull()) {
        // save to cache
        SynoImageProviderPrivate::CacheLocker cacheLocker(m_provider->d_func());
        cacheLocker.cache().insert(m_id, m_image);
    } else {
        QString readerError = reader.errorString();
        if (!readerError.isEmpty()) {
            setErrorString(tr("Decoding error: %1.").arg(readerError));
        } else {
            setErrorString(tr("Unknown decoding error."));
        }

        m_image = QImage();
    }
}

void SynoImageResponse::postProcessImage()
{
    // scale image if requested
    if (!m_image.isNull() && m_size.isValid() && !m_size.isNull()) {
        // set minimum dimensions to (1, 1) in case of one of axes is not specified
        QSize sz = m_size.expandedTo(QSize(1, 1));
        m_image = m_image.scaled(sz, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }

    ColorHandler::convertColorSpace(m_image, m_image.colorSpace(), m_options.targetColorSpace());
}

QByteArray SynoImageResponse::synoThumbSize() const
{
    SynoSizeGadget::SynoSize synoSize = SynoSizeGadget::instance().fitSyno(m_size.height(), m_size.width());
    if (synoSize <= SynoSizeGadget::SIZE_M) {
        return QByteArrayLiteral("small");
    }

    return QByteArrayLiteral("large");
}

void SynoImageResponse::onCacheCheckFinished(bool success)
{
    if (success) {
        emit finished();
    } else {
        CancelStatus cancel(Status_Cancelled);
        if (!m_cancelStatus.compare_exchange_strong(cancel, Statuc_CancelledConfirmed)) {
            sendRequest();
        } else {
            emit finished();
        }
    }
}

SynoImageProviderPrivate::CacheLocker::CacheLocker(SynoImageProviderPrivate* d)
    : m_d(d)
{
    m_d->imageCacheMutex.lock();
}

SynoImageProviderPrivate::CacheLocker::~CacheLocker()
{
    m_d->imageCacheMutex.unlock();
}

SynoImageCache& SynoImageProviderPrivate::CacheLocker::cache()
{
    return m_d->imageCache;
}
