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
#include "synoconn.h"
#include "synops.h"
#include "synoreplyjson.h"

#include <QBuffer>
#include <QImageReader>
#include <QQuickTextureFactory>
#include <QTimer>

#include <functional>

SynoImageProvider::SynoImageProvider(SynoConn* conn)
    : QObject()
    , QQuickAsyncImageProvider()
    , m_conn(conn)
{
    QTimer* cacheStatisticTimer = new QTimer(this);
    connect(cacheStatisticTimer, &QTimer::timeout, this, [this]() {
        SynoImageProvider::CacheLocker cacheLocker(this);
        SynoImageCache& cache = cacheLocker.cache();
        qDebug() << tr("Image cache statistics. Count: %1. Cost (KB): %2. Hit: %3. Miss: %4.")
                    .arg(cache.count()).arg(cache.totalCost() / 1024)
                    .arg(cache.hitCount()).arg(cache.missCount());
    });
    cacheStatisticTimer->start(30000);
}

SynoConn* SynoImageProvider::conn() const
{
    return m_conn;
}

void SynoImageProvider::invalidateInCache(const QString& id)
{
    SynoImageProvider::CacheLocker cacheLocker(this);
    cacheLocker.cache().remove(id);
}

QQuickImageResponse* SynoImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    SynoImageResponse* response = new SynoImageResponse(this, id.toLatin1(), requestedSize);
    response->load();
    return response;
}

SynoImageResponse::SynoImageResponse(SynoImageProvider* provider, const QByteArray& id, const QSize& size)
    : m_provider(provider)
    , m_id(id)
    , m_size(size)
{
}

void SynoImageResponse::load()
{
    if (loadFromCache()) {
        postProcessImage();
        QTimer::singleShot(0, this, std::bind(&SynoImageResponse::finished, this));
    } else {
        sendRequest();
    }
}

void SynoImageResponse::sendRequest()
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=get");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("size=") + synoSizeForQSize(m_size);
    formData << QByteArrayLiteral("id=") + m_id;

    Q_ASSERT(!m_req);
    m_req = m_provider->conn()->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Thumb"), formData);
    m_req->send(this, [this] {
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
                processNetworkRequest();
                postProcessImage();
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
    Q_ASSERT(m_req);
    m_req->cancel();
    emit finished();
}

void SynoImageResponse::setErrorString(const QString& err)
{
    m_errorString = tr("Unable to obtain image. %1\nId: %2")
                    .arg(err).arg(QString::fromLatin1(m_id));
    qWarning() << __FUNCTION__ << m_errorString;
}

bool SynoImageResponse::loadFromCache()
{
    SynoImageProvider::CacheLocker cacheLocker(m_provider);
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
        SynoImageProvider::CacheLocker cacheLocker(m_provider);
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
}

QByteArray SynoImageResponse::synoSizeForQSize(const QSize& size)
{
    // TBD: implement
    Q_UNUSED(size)
    return QByteArrayLiteral("small");
}

SynoImageProvider::CacheLocker::CacheLocker(SynoImageProvider* provider)
    : m_provider(provider)
{
    m_provider->m_imageCacheMutex.lock();
}

SynoImageProvider::CacheLocker::~CacheLocker()
{
    m_provider->m_imageCacheMutex.unlock();
}

SynoImageCache& SynoImageProvider::CacheLocker::cache()
{
    return m_provider->m_imageCache;
}
